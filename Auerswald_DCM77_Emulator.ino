#include "config.h"
#include "ntp.h"
#include "dcf77.h"
#include "httpserver.h"
#include "watchdog.h"

#include <Arduino.h>
#include <time.h>


void setup()
{
    // Initialize the DCF77 output
    initDCF();

    if (DEBUG_SERIAL) Serial.begin(115200);

    // Connect to WiFi or start access point
    bool connected = connectWiFi();

    // Always start the web server
    initHttpServer();

    // NTP only when a WIFI connection is active
    if (connected)
    {
        initNTP();
        waitForTime();
    }

    initNtpWatchdog();
}


void loop()
{
    struct tm now;

    // DCF77 may transmit only when NTP synchronization is active.
    if (!isDcf77TransmissionAllowed())
    {
        dcfInactive();
        delay(100);
        return;
    }

    // An abort event always belongs to the previous telegram.
    // A new telegram cycle is now beginning.
    clearDcf77TransmissionAbortRequest();

    if (!getLocalTime(&now))
    {
        dcfInactive();
        delay(100);
        return;
    }

    // Generate a DCF77 telegram
    createDCF77Telegram(now);

    // Wait for the next minute to start.
    waitForNextMinute();

    // While waiting, the watchdog may cause the NTP to become invalid.
    // In this case, DO NOT start a telegram; instead, immediately return to the next
    // loop() iteration.
    if (!isDcf77TransmissionAllowed() ||
        isDcf77TransmissionAbortRequested())
    {
        dcfInactive();
        return;
    }

    // Transmit DCF77 data for 59 seconds.
    for (int second = 0; second < 59; second++)
    {
        if (dcfBits[second])
            sendBit1();
        else
            sendBit0();

        // The watchdog may interrupt a telegram in progress at any time.
        // After the return statement, loop() starts a new cycle.
        if (!isDcf77TransmissionAllowed() ||
            isDcf77TransmissionAbortRequested())
        {
            dcfInactive();
            return;
        }
    }
}
