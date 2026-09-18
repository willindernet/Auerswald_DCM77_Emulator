#ifndef NTP_H
#define NTP_H

#include <Arduino.h>

//---------------------------------------------------------------------------
// bool connectWiFi()
//---------------------------------------------------------------------------
// Description     | Connect to WIFI
// Parameter       | None
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
bool connectWiFi();


//---------------------------------------------------------------------------
// void initNTP()
//---------------------------------------------------------------------------
// Description     | Configure NTP
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void initNTP();


//---------------------------------------------------------------------------
// void waitForTime()
//---------------------------------------------------------------------------
// Description     | Wait for the valid time
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void waitForTime();


//---------------------------------------------------------------------------
// bool isWiFiConnected()
//---------------------------------------------------------------------------
// Description     | WIFI conection status
// Parameter       | None
// Return value    | bool: 1 -> connected
//                 | bool: 0 -> not connected
//---------------------------------------------------------------------------
bool isWiFiConnected();


//---------------------------------------------------------------------------
// bool isAccessPointMode()
//---------------------------------------------------------------------------
// Description     | check access point mode
// Parameter       | None
// Return value    | bool: 1 -> access point mode
//                 | bool: 0 -> not access point mode
//---------------------------------------------------------------------------
bool isAccessPointMode();


//---------------------------------------------------------------------------
// bool hasWiFiCredentials()
//---------------------------------------------------------------------------
// Description     | check for WIFI credentials available
// Parameter       | None
// Return value    | bool: 1 -> credentials available
//                 | bool: 0 -> no credentials available
//---------------------------------------------------------------------------
bool hasWiFiCredentials();


//---------------------------------------------------------------------------
// bool isTimeValid()
//---------------------------------------------------------------------------
// Description     | Validity of the time source
// Parameter       | None
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
bool isTimeValid();


//---------------------------------------------------------------------------
// bool isNtpSynchronizationFresh()
//---------------------------------------------------------------------------
// Description     | NTP synchronization status for the watchdog
// Parameter       | None
// Return value    | bool: 1 -> last NTP synchronisation in configured time window
//                 | bool: 0 -> no NTP synchronisation in configured time window
//---------------------------------------------------------------------------
bool isNtpSynchronizationFresh();


//---------------------------------------------------------------------------
// bool refreshNtpSynchronization()
//---------------------------------------------------------------------------
// Description     | refresh NTP synchronization
// Parameter       | None
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
bool refreshNtpSynchronization();


//---------------------------------------------------------------------------
// void saveWiFiCredentials(const String &ssid, const String &password)
//---------------------------------------------------------------------------
// Description     | Save WIFI login information
// Parameter       | String ssid: WIFI ssid
// Parameter       | String password: WIFI password
// Return value    | void
//---------------------------------------------------------------------------
void saveWiFiCredentials(const String &ssid, const String &password);


//---------------------------------------------------------------------------
// void clearWiFiCredentials()
//---------------------------------------------------------------------------
// Description     | Delete WIFI login information
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void clearWiFiCredentials();


//---------------------------------------------------------------------------
// void loadTimeConfiguration()
//---------------------------------------------------------------------------
// Description     | Save time configuration
// Parameter       | String server1: Address of NTP server
// Parameter       | String server2: Address of NTP server
// Parameter       | String timezone: time zone
// Return value    | void
//---------------------------------------------------------------------------
void loadTimeConfiguration(String &server1, String &server2, String &timezone);


//---------------------------------------------------------------------------
// void saveTimeConfiguratio n(const String &server1, const String &server2, const String &timezone)
//---------------------------------------------------------------------------
// Description     | Load WIFI login credentials from NVS // ------------------------------------------------------------
// Parameter       | server1: const String &
// Parameter       | server2: const String &
// Parameter       | timezone: const String &
// Return value    | The void saveTimeConfiguratio result of the operation.
//---------------------------------------------------------------------------
void saveTimeConfiguration(const String &server1, const String &server2, const String &timezone);


//---------------------------------------------------------------------------
// void resetTimeConfiguration()
//---------------------------------------------------------------------------
// Description     | Reset the time settings to factory defaults
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void resetTimeConfiguration();


//---------------------------------------------------------------------------
// void getTimeConfiguration()
//---------------------------------------------------------------------------
// Description     | Provide the current time configuration
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void getTimeConfiguration(String &server1, String &server2, String &timezone);

#endif
