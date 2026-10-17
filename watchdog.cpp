#include "watchdog.h"
#include "ntp.h"
#include "dcf77.h"
#include "config.h"

#include <Arduino.h>

static TaskHandle_t ntpWatchdogTaskHandle = nullptr;
static volatile bool dcf77TransmissionAllowed = false;
static volatile bool dcf77TransmissionAbortRequested = false;

//---------------------------------------------------------------------------
// void tryNtpRecovery()
//---------------------------------------------------------------------------
// Description     | try to reconnect NTP
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
static bool ntpExpiryReported = false;
static bool ntpRecoveryFailureReported = false;

static void tryNtpRecovery()
{
    // NTP is no longer fresh: stop DCF77 output and abort any active telegram.
    dcf77TransmissionAllowed = false;
    dcf77TransmissionAbortRequested = true;
    dcfInactive();

#if STATUS_LOGGING
    if (!ntpExpiryReported)
    {
        Serial.println("[WATCHDOG] NTP synchronization expired; DCF77 transmission stopped");
        ntpExpiryReported = true;
    }
#endif

    // Wi-Fi reconnection is handled once, centrally, in ntpWatchdogTask().
    // This function only attempts NTP synchronization when Wi-Fi is available.
    if (isWiFiConnected() && refreshNtpSynchronization())
    {
        dcf77TransmissionAllowed = true;
        ntpExpiryReported = false;
        ntpRecoveryFailureReported = false;

#if STATUS_LOGGING
        Serial.println("[WATCHDOG] NTP recovery successful; DCF77 will resume with the next telegram");
#endif
    }
    else
    {
#if STATUS_LOGGING
        if (!ntpRecoveryFailureReported)
        {
            Serial.println("[WATCHDOG] NTP recovery failed; DCF77 remains stopped");
            ntpRecoveryFailureReported = true;
        }
#endif
    }
}

//---------------------------------------------------------------------------
// void ntpWatchdogTask(void *parameter)
//---------------------------------------------------------------------------
// Description     | create watchdog task
// Parameter       | parameter: void *
// Return value    | void
//---------------------------------------------------------------------------
static void ntpWatchdogTask(void *parameter)
{
    (void)parameter;

    for (;;)
    {
        // Retry Wi-Fi independently of the NTP freshness timeout. This lets
        // the device recover promptly when the router becomes available again.
        if (!isWiFiConnected() && hasWiFiCredentials())
        {
            // Reconnect in one place only. If NTP is stale, the recovery
            // branch below will perform exactly one synchronization attempt.
            reconnectWiFi();
        }

        if (isNtpSynchronizationFresh())
        {
            if (!dcf77TransmissionAllowed)
            {
                dcf77TransmissionAllowed = true;
            }
        }
        else
        {
            tryNtpRecovery();
        }

        vTaskDelay(pdMS_TO_TICKS(NTP_WATCHDOG_INTERVAL * 60000UL));
    }
}


//---------------------------------------------------------------------------
// void initNtpWatchdog()
//---------------------------------------------------------------------------
// Description     | init watchdog task
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void initNtpWatchdog()
{
    if (ntpWatchdogTaskHandle != nullptr)
        return;

    BaseType_t result = xTaskCreatePinnedToCore(
        ntpWatchdogTask, "NTPWatchdog", 4096, nullptr, 1,
        &ntpWatchdogTaskHandle, 0
    );

    if (result != pdPASS)
    {
        ntpWatchdogTaskHandle = nullptr;
#if STATUS_LOGGING
        Serial.println("[WATCHDOG] ERROR: failed to create watchdog task");
#endif
    }
#if STATUS_LOGGING
    else
    {
        Serial.println("[WATCHDOG] Task started");
    }
#endif
}


//---------------------------------------------------------------------------
// bool isDcf77TransmissionAllowed()
//---------------------------------------------------------------------------
// Description     | Check whether DCF77 transmission is possible
// Parameter       | None
// Return value    | bool: 1 -> DCF77 transmission is possible
//                 | bool: 0 -> DCF77 transmission is not possible
//---------------------------------------------------------------------------
bool isDcf77TransmissionAllowed()
{
    return dcf77TransmissionAllowed;
}


//---------------------------------------------------------------------------
// bool isDcf77TransmissionAbortRequested()
//---------------------------------------------------------------------------
// Description     | Check whether DCF77 transmission is requested
// Parameter       | None
// Return value    | bool: 1 -> DCF77 transmission is requested
//                 | bool: 0 -> DCF77 transmission is not requested
//---------------------------------------------------------------------------
bool isDcf77TransmissionAbortRequested()
{
    return dcf77TransmissionAbortRequested;
}


//---------------------------------------------------------------------------
// void clearDcf77TransmissionAbortRequest()
//---------------------------------------------------------------------------
// Description     | clear DCF77 transmission abort request
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void clearDcf77TransmissionAbortRequest()
{
    dcf77TransmissionAbortRequested = false;
}
