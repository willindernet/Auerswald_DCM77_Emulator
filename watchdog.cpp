#include "watchdog.h"
#include "ntp.h"
#include "dcf77.h"
#include "config.h"

#include <Arduino.h>

static TaskHandle_t ntpWatchdogTaskHandle = nullptr;
static volatile bool dcf77TransmissionAllowed = false;
static volatile bool dcf77TransmissionAbortRequested = false;

static void tryNtpRecovery()
{
    // NTP ist nicht mehr frisch: DCF77 sofort sperren.
    dcf77TransmissionAllowed = false;
    dcf77TransmissionAbortRequested = true;
    dcfInactive();

#if DEBUG_SERIAL
    Serial.print("[WATCHDOG] NTP TIMEOUT -> DCF77 OFF, uptime=");
    Serial.print(millis() / 1000UL);
    Serial.println(" s");
#endif

    if (!isWiFiConnected() && hasWiFiCredentials())
        connectWiFi();

    if (isWiFiConnected() && refreshNtpSynchronization())
    {
        // Das Abbruchflag bleibt absichtlich gesetzt. Damit kann ein
        // laufendes Telegramm nach einem Timeout niemals fortgesetzt werden.
        // Erst der nächste loop()-Zyklus löscht es vor einem neuen Telegramm.
        dcf77TransmissionAllowed = true;

#if DEBUG_SERIAL
        Serial.print("[WATCHDOG] NTP RECOVERY OK -> DCF77 READY, uptime=");
        Serial.print(millis() / 1000UL);
        Serial.println(" s");
#endif
    }
    else
    {
#if DEBUG_SERIAL
        Serial.print("[WATCHDOG] NTP RECOVERY FAILED, DCF77 OFF, uptime=");
        Serial.print(millis() / 1000UL);
        Serial.println(" s");
#endif
    }
}

static void ntpWatchdogTask(void *parameter)
{
    (void)parameter;

    for (;;)
    {
        if (isNtpSynchronizationFresh())
        {
            if (!dcf77TransmissionAllowed)
            {
                dcf77TransmissionAllowed = true;
#if DEBUG_SERIAL
                Serial.print("[WATCHDOG] NTP fresh -> DCF77 allowed, uptime=");
                Serial.print(millis() / 1000UL);
                Serial.println(" s");
#endif
            }
        }
        else
        {
            tryNtpRecovery();
        }

        vTaskDelay(pdMS_TO_TICKS(NTP_WATCHDOG_INTERVAL_MS));
    }
}

void initNtpWatchdog()
{
    if (ntpWatchdogTaskHandle != nullptr)
        return;

    xTaskCreatePinnedToCore(
        ntpWatchdogTask, "NTPWatchdog", 4096, nullptr, 1,
        &ntpWatchdogTaskHandle, 0
    );
}

bool isDcf77TransmissionAllowed()
{
    return dcf77TransmissionAllowed;
}

bool isDcf77TransmissionAbortRequested()
{
    return dcf77TransmissionAbortRequested;
}

void clearDcf77TransmissionAbortRequest()
{
    dcf77TransmissionAbortRequested = false;
}
