#include "dcf77.h"
#include "config.h"

#include <Arduino.h>
#include <esp_timer.h>
#include <string.h>
#include <time.h>


// --------------------------------------------------
// Array for time information
// --------------------------------------------------

bool dcfBits[59];


// --------------------------------------------------
// DCF Initialization
// --------------------------------------------------

void initDCF()
{
    pinMode(DCF_PIN, OUTPUT);

    // Leitung inaktiv
    digitalWrite(DCF_PIN, LOW);
}


// --------------------------------------------------
// Wait for the next minute to start
// --------------------------------------------------

void waitForNextMinute()
{
    // Wait until the seconds reach “00”
    struct tm now;

    if (!getLocalTime(&now))
        return;

    // Current time as Unix time
    time_t t = mktime(&now);

    // Time of the next minute change
    t += 60 - now.tm_sec;

    // Current ESP32 time in microseconds
    int64_t startUs = esp_timer_get_time();

    // Time remaining until the next minute changes
    // in microseconds
    int64_t waitUs =
        (t - time(nullptr)) * 1000000LL;

    // First, wait for a short while so that the CPU
    // isn't unnecessarily occupied
    if (waitUs > 20000)
    {
        delay((waitUs - 10000) / 1000);
    }

    // Wait for the minute to change in high resolution
    while (esp_timer_get_time() - startUs < waitUs)
    {
        // intentionally left blank
    }
}


// --------------------------------------------------
// Transmit DCF77 Bit 1
// --------------------------------------------------

void sendBit1()
{
    // Send a "1"
    // 200 ms high, 800 ms low

    dcfActive();

    delay(200);

    dcfInactive();

    delay(800);

    if (DEBUG_SERIAL) Serial.print("1");
}


// --------------------------------------------------
// Transmit DCF77 Bit 0
// --------------------------------------------------

void sendBit0()
{
    // Send a "0"
    // 100 ms high, 900 ms low

    dcfActive();

    delay(100);

    dcfInactive();

    delay(900);

    if (DEBUG_SERIAL) Serial.print("0");
}


// --------------------------------------------------
// DCF output active
// --------------------------------------------------

void dcfActive()
{
    // Set the GPIO pin to 1
    digitalWrite(DCF_PIN, HIGH);
}


// --------------------------------------------------
// DCF output inactive
// --------------------------------------------------

void dcfInactive()
{
    // Set the GPIO pin to 0
    digitalWrite(DCF_PIN, LOW);
}


// --------------------------------------------------
// Parity calculation
// --------------------------------------------------

int parity(int start, int end)
{
    // Calculating Parity
    // An even number of "1"s is always transmitted

    int count = 0;

    for (int i = start; i <= end; i++)
    {
        count += dcfBits[i];
    }

    return count % 2;
}


// --------------------------------------------------
// Calculation of the summer time announcement
// --------------------------------------------------

bool getDCF77A1(const struct tm &t)
{
    // ----------------------------------------
    // The last sunday in march
    // ----------------------------------------

    int marchLastDay = 31;

    struct tm marchLast = {};

    marchLast.tm_year = t.tm_year;
    marchLast.tm_mon  = 2;           // March
    marchLast.tm_mday = marchLastDay;
    marchLast.tm_hour = 12;

    mktime(&marchLast);

    int marchLastSunday =
        marchLastDay - marchLast.tm_wday;


    // ----------------------------------------
    // The last sunday in october
    // ----------------------------------------

    int octoberLastDay = 31;

    struct tm octoberLast = {};

    octoberLast.tm_year = t.tm_year;
    octoberLast.tm_mon  = 9;         // October
    octoberLast.tm_mday = octoberLastDay;
    octoberLast.tm_hour = 12;

    mktime(&octoberLast);

    int octoberLastSunday =
        octoberLastDay - octoberLast.tm_wday;


    // ----------------------------------------
    // Change from CET to CEST
    // Announcement Time: 1:00 – 1:59 CET
    // ----------------------------------------

    if (t.tm_mon == 2 &&
        t.tm_mday == marchLastSunday &&
        t.tm_hour == 1 &&
        t.tm_isdst == 0)
    {
        return true;
    }


    // ----------------------------------------
    // Change from CEST to CET
    // Announcement Time: 2:00 – 2:59 CEST
    // ----------------------------------------

    if (t.tm_mon == 9 &&
        t.tm_mday == octoberLastSunday &&
        t.tm_hour == 2 &&
        t.tm_isdst > 0)
    {
        return true;
    }

    return false;
}


// --------------------------------------------------
// Generate DCF77 telegram
// --------------------------------------------------

void createDCF77Telegram(struct tm now)
{
    // Setting the individual bits of the time message

    // The bit for the leap second is not sent
    memset(dcfBits, 0, sizeof(dcfBits));


    // ------------------------------------------------
    // Time of the next minute
    // ------------------------------------------------

    // The time for the next minute is always 
    // transmitted: +60
    //
    // waitForNextMinute( ) triggers the
    // start of a new minute: +60

    time_t t = mktime(&now);

    t += 120;

    localtime_r(&t, &now);


    // ------------------------------------------------
    // Time variables
    // ------------------------------------------------

    int minute = now.tm_min;
    int hour   = now.tm_hour;
    int day    = now.tm_mday;
    int month  = now.tm_mon + 1;
    int year   = now.tm_year % 100;
    bool summertime = now.tm_isdst > 0;
    bool summerwinterchange = getDCF77A1(now);
    int weekday = now.tm_wday;
    if (weekday == 0) weekday = 7;


    // ------------------------------------------------
    // Bit 0
    // ------------------------------------------------

    // Minute mark, always 0 bit

    dcfBits[0] = 0;


    // ------------------------------------------------
    // Bit 1-14
    // ------------------------------------------------

    // Weather data

    dcfBits[1]  = 0;
    dcfBits[2]  = 0;
    dcfBits[3]  = 0;
    dcfBits[4]  = 0;
    dcfBits[5]  = 0;
    dcfBits[6]  = 0;
    dcfBits[7]  = 0;
    dcfBits[8]  = 0;
    dcfBits[9]  = 0;
    dcfBits[10] = 0;
    dcfBits[11] = 0;
    dcfBits[12] = 0;
    dcfBits[13] = 0;
    dcfBits[14] = 0;


    // ------------------------------------------------
    // Bit 15
    // ------------------------------------------------

    // Rufbit
    // If it is 1, there may be a malfunction

    dcfBits[15] = 0;


    // ------------------------------------------------
    // Bit 16
    // ------------------------------------------------

    // Announcement of the time change

    dcfBits[16] = summerwinterchange;


    // ------------------------------------------------
    // Bit 17 / 18
    // ------------------------------------------------

    // Summer time: 10
    // Winter time: 01

    dcfBits[17] = summertime;
    dcfBits[18] = !summertime;


    // ------------------------------------------------
    // Bit 19
    // ------------------------------------------------

    // Announcement of a leap second

    dcfBits[19] = 0;


    // ------------------------------------------------
    // Bit 20
    // ------------------------------------------------

    // Start time information

    dcfBits[20] = 1;


    // =================================================
    // Minute
    // =================================================

    dcfBits[21] = ((minute % 10) & 1) ? 1 : 0;
    dcfBits[22] = ((minute % 10) & 2) ? 1 : 0;
    dcfBits[23] = ((minute % 10) & 4) ? 1 : 0;
    dcfBits[24] = ((minute % 10) & 8) ? 1 : 0;
    dcfBits[25] = ((minute / 10) & 1) ? 1 : 0;
    dcfBits[26] = ((minute / 10) & 2) ? 1 : 0;
    dcfBits[27] = ((minute / 10) & 4) ? 1 : 0;


    // Parity for Minute

    dcfBits[28] = parity(21, 27);


    // =================================================
    // Hour
    // =================================================

    dcfBits[29] = ((hour % 10) & 1) ? 1 : 0;
    dcfBits[30] = ((hour % 10) & 2) ? 1 : 0;
    dcfBits[31] = ((hour % 10) & 4) ? 1 : 0;
    dcfBits[32] = ((hour % 10) & 8) ? 1 : 0;
    dcfBits[33] = ((hour / 10) & 1) ? 1 : 0;
    dcfBits[34] = ((hour / 10) & 2) ? 1 : 0;


    // Parity for hour

    dcfBits[35] = parity(29, 34);


    // =================================================
    // Day
    // =================================================

    dcfBits[36] = ((day % 10) & 1) ? 1 : 0;
    dcfBits[37] = ((day % 10) & 2) ? 1 : 0;
    dcfBits[38] = ((day % 10) & 4) ? 1 : 0;
    dcfBits[39] = ((day % 10) & 8) ? 1 : 0;
    dcfBits[40] = ((day / 10) & 1) ? 1 : 0;
    dcfBits[41] = ((day / 10) & 2) ? 1 : 0;


    // =================================================
    // Weekday
    // =================================================

    dcfBits[42] = (weekday & 1) ? 1 : 0;
    dcfBits[43] = (weekday & 2) ? 1 : 0;
    dcfBits[44] = (weekday & 4) ? 1 : 0;


    // =================================================
    // Month
    // =================================================

    dcfBits[45] = ((month % 10) & 1) ? 1 : 0;
    dcfBits[46] = ((month % 10) & 2) ? 1 : 0;
    dcfBits[47] = ((month % 10) & 4) ? 1 : 0;
    dcfBits[48] = ((month % 10) & 8) ? 1 : 0;
    dcfBits[49] = ((month / 10) & 1) ? 1 : 0;


    // =================================================
    // Year
    // =================================================

    dcfBits[50] = ((year % 10) & 1) ? 1 : 0;
    dcfBits[51] = ((year % 10) & 2) ? 1 : 0;
    dcfBits[52] = ((year % 10) & 4) ? 1 : 0;
    dcfBits[53] = ((year % 10) & 8) ? 1 : 0;
    dcfBits[54] = ((year / 10) & 1) ? 1 : 0;
    dcfBits[55] = ((year / 10) & 2) ? 1 : 0;
    dcfBits[56] = ((year / 10) & 4) ? 1 : 0;
    dcfBits[57] = ((year / 10) & 8) ? 1 : 0;


    // =================================================
    // Parity
    // =================================================

    // Parity for day, weekday, month, year

    dcfBits[58] = parity(36, 57);
}