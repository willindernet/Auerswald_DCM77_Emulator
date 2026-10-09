#include "config.h"
#include "ntp.h"
#include "dcf77.h"
#include "httpserver.h"
#include "watchdog.h"
#include "version.h"

#include <Arduino.h>
#include <time.h>


void setup()
{
#if STATUS_LOGGING
    Serial.begin(115200);
    Serial.printf("\n[START] Auerswald DCM77 Emulator v%s\n", PROJECT_VERSION);
#endif
    // Initialize the DCF77 output
    initDCF();


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

    // Flash the DCF77 output while the ESP32 is in access point mode.
    // The 100 ms / 100 ms pattern indicates that the ESP32 is not connected
    // to the configured WiFi network.
    if (isAccessPointMode())
    {
        accessPointBlink();
        return;
    }

    accessPointIndicatorReset();

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
#if STATUS_LOGGING
    Serial.printf("[TIME] %04d-%02d-%02d %02d:%02d:%02d\n",
                  now.tm_year + 1900, now.tm_mon + 1, now.tm_mday,
                  now.tm_hour, now.tm_min, now.tm_sec);
    Serial.println("[DCF77] Generating telegram");
#endif
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
#if STATUS_LOGGING
    Serial.println("[DCF77] Transmission started");
#endif
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
