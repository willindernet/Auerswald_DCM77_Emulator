#include "dcf77.h"
#include "timezone.h"
#include "config.h"

#include <Arduino.h>
#include <esp_timer.h>
#include <string.h>


// --------------------------------------------------
// Array für Zeitinformation
// --------------------------------------------------

bool dcfBits[59];


// --------------------------------------------------
// DCF Initialisierung
// --------------------------------------------------

void initDCF()
{
    pinMode(DCF_PIN, OUTPUT);

    // Leitung inaktiv
    digitalWrite(DCF_PIN, LOW);
}


// --------------------------------------------------
// Auf nächsten Minutenwechsel warten
// --------------------------------------------------

void waitForNextMinute()
{
    // Warten, bis die Sekunde "00" erreicht wird
    struct tm now;

    if (!getLocalTime(&now))
        return;

    // Aktuelle Zeit als Unix-Zeit
    time_t t = mktime(&now);

    // Zeitpunkt des nächsten Minutenwechsels
    t += 60 - now.tm_sec;

    // Aktuelle ESP32-Zeit in Mikrosekunden
    int64_t startUs = esp_timer_get_time();

    // Dauer bis zum nächsten Minutenwechsel
    // in Mikrosekunden
    int64_t waitUs =
        (t - time(nullptr)) * 1000000LL;

    // Zunächst grob warten, damit die CPU
    // nicht unnötig beschäftigt wird
    if (waitUs > 20000)
    {
        delay((waitUs - 10000) / 1000);
    }

    // Jetzt mit hoher Auflösung auf den
    // Minutenwechsel warten
    while (esp_timer_get_time() - startUs < waitUs)
    {
        // bewusst leer
    }
}


// --------------------------------------------------
// DCF77 Bit 1 senden
// --------------------------------------------------

void sendBit1()
{
    // Sende eine "1"
    // 200 ms high, 800 ms low

    dcfActive();

    delay(200);

    dcfInactive();

    delay(800);

    Serial.print("1");
}


// --------------------------------------------------
// DCF77 Bit 0 senden
// --------------------------------------------------

void sendBit0()
{
    // Sende eine "0"
    // 100 ms high, 900 ms low

    dcfActive();

    delay(100);

    dcfInactive();

    delay(900);

    Serial.print("0");
}


// --------------------------------------------------
// DCF Ausgang aktiv
// --------------------------------------------------

void dcfActive()
{
    // Setze GPIO Pin auf 1
    digitalWrite(DCF_PIN, HIGH);
}


// --------------------------------------------------
// DCF Ausgang inaktiv
// --------------------------------------------------

void dcfInactive()
{
    // Setze GPIO Pin auf 0
    digitalWrite(DCF_PIN, LOW);
}


// --------------------------------------------------
// Paritätsberechnung
// --------------------------------------------------

int parity(int start, int end)
{
    // Berechnen der Parity
    // Es wird immer eine gerade Anzahl von "1"
    // gesendet

    int count = 0;

    for (int i = start; i <= end; i++)
    {
        count += dcfBits[i];
    }

    return count % 2;
}


// --------------------------------------------------
// DCF77 Telegramm erzeugen
// --------------------------------------------------

void createDCF77Telegram(struct tm now)
{
    // Setzen der einzelnen Bits der Zeitnachricht
    // Kommentare für detailliertes Tracing

    // Bit für Schaltsekunde wird nicht geschickt
    memset(dcfBits, 0, sizeof(dcfBits));


    // ------------------------------------------------
    // Zeit der folgenden Minute
    // ------------------------------------------------

    // Es wird immer die Zeit der folgenden Minute
    // geschickt: +60
    //
    // Durch waitForNextMinute() wird ein
    // Minutenwechsel herbeigeführt: +60

    time_t t = mktime(&now);

    t += 120;

    localtime_r(&t, &now);


    // ------------------------------------------------
    // Zeitvariablen
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

    // Minutenmarke, immer 0-Bit

    dcfBits[0] = 0;


    // ------------------------------------------------
    // Bit 1-14
    // ------------------------------------------------

    // Wetterdaten

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
    // Ist es 1, liegt ggf. eine Störung vor

    dcfBits[15] = 0;


    // ------------------------------------------------
    // Bit 16
    // ------------------------------------------------

    // Ankündigung Zeitumstellung

    dcfBits[16] = summerwinterchange;


    // ------------------------------------------------
    // Bit 17 / 18
    // ------------------------------------------------

    // Sommerzeit: 10
    // Winterzeit: 01

    dcfBits[17] = summertime;
    dcfBits[18] = !summertime;


    // ------------------------------------------------
    // Bit 19
    // ------------------------------------------------

    // Ankündigung einer Schaltsekunde

    dcfBits[19] = 0;


    // ------------------------------------------------
    // Bit 20
    // ------------------------------------------------

    // Start Zeitinformation

    dcfBits[20] = 1;


    // =================================================
    // MINUTE
    // =================================================

    dcfBits[21] = ((minute % 10) & 1) ? 1 : 0;
    dcfBits[22] = ((minute % 10) & 2) ? 1 : 0;
    dcfBits[23] = ((minute % 10) & 4) ? 1 : 0;
    dcfBits[24] = ((minute % 10) & 8) ? 1 : 0;
    dcfBits[25] = ((minute / 10) & 1) ? 1 : 0;
    dcfBits[26] = ((minute / 10) & 2) ? 1 : 0;
    dcfBits[27] = ((minute / 10) & 4) ? 1 : 0;


    // Parity für Minute

    dcfBits[28] = parity(21, 27);


    // =================================================
    // STUNDE
    // =================================================

    dcfBits[29] = ((hour % 10) & 1) ? 1 : 0;
    dcfBits[30] = ((hour % 10) & 2) ? 1 : 0;
    dcfBits[31] = ((hour % 10) & 4) ? 1 : 0;
    dcfBits[32] = ((hour % 10) & 8) ? 1 : 0;
    dcfBits[33] = ((hour / 10) & 1) ? 1 : 0;
    dcfBits[34] = ((hour / 10) & 2) ? 1 : 0;


    // Parity für Stunde

    dcfBits[35] = parity(29, 34);


    // =================================================
    // TAG
    // =================================================

    dcfBits[36] = ((day % 10) & 1) ? 1 : 0;
    dcfBits[37] = ((day % 10) & 2) ? 1 : 0;
    dcfBits[38] = ((day % 10) & 4) ? 1 : 0;
    dcfBits[39] = ((day % 10) & 8) ? 1 : 0;
    dcfBits[40] = ((day / 10) & 1) ? 1 : 0;
    dcfBits[41] = ((day / 10) & 2) ? 1 : 0;


    // =================================================
    // WOCHENTAG
    // =================================================

    dcfBits[42] = (weekday & 1) ? 1 : 0;
    dcfBits[43] = (weekday & 2) ? 1 : 0;
    dcfBits[44] = (weekday & 4) ? 1 : 0;


    // =================================================
    // MONAT
    // =================================================

    dcfBits[45] = ((month % 10) & 1) ? 1 : 0;
    dcfBits[46] = ((month % 10) & 2) ? 1 : 0;
    dcfBits[47] = ((month % 10) & 4) ? 1 : 0;
    dcfBits[48] = ((month % 10) & 8) ? 1 : 0;
    dcfBits[49] = ((month / 10) & 1) ? 1 : 0;


    // =================================================
    // JAHR
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
    // PARITY
    // =================================================

    // Parity für Tag, Wochentag, Monat, Jahr

    dcfBits[58] = parity(36, 57);
}