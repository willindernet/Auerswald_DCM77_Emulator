#ifndef NTP_H
#define NTP_H

#include <Arduino.h>

bool connectWiFi();
void initNTP();
void waitForTime();

bool isWiFiConnected();
bool isAccessPointMode();
bool hasWiFiCredentials();
bool isTimeValid();

// Cyclic NTP security monitoring
bool isNtpSynchronizationFresh();
bool refreshNtpSynchronization();

void saveWiFiCredentials(const String &ssid, const String &password);
void clearWiFiCredentials();

// NTP / time zone
void loadTimeConfiguration(String &server1, String &server2, String &timezone);
void saveTimeConfiguration(const String &server1, const String &server2, const String &timezone);
void resetTimeConfiguration();
void getTimeConfiguration(String &server1, String &server2, String &timezone);

#endif
