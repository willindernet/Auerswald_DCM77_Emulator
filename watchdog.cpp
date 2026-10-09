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
static void tryNtpRecovery()
{
    // NTP is no longer up to date: Disable DCF77 immediately.
    dcf77TransmissionAllowed = false;
    dcf77TransmissionAbortRequested = true;
    dcfInactive();
#if STATUS_LOGGING
    Serial.println("[WATCHDOG] NTP synchronization expired; DCF77 transmission stopped");
#endif

    if (!isWiFiConnected() && hasWiFiCredentials())
        connectWiFi();

    if (isWiFiConnected() && refreshNtpSynchronization())
    {
        // The abort flag is intentionally left set. This ensures that a
        // telegram currently in progress can never be resumed after a timeout.
        // Only the next loop() cycle clears it before a new telegram.
        dcf77TransmissionAllowed = true;
#if STATUS_LOGGING
        Serial.println("[WATCHDOG] NTP recovery successful; DCF77 transmission re-enabled for the next telegram");
#endif

    }
    else
    {
#if STATUS_LOGGING
        Serial.println("[WATCHDOG] NTP recovery failed; DCF77 remains stopped");
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

    xTaskCreatePinnedToCore(
        ntpWatchdogTask, "NTPWatchdog", 4096, nullptr, 1,
        &ntpWatchdogTaskHandle, 0
    );
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
