#ifndef NTP_H
#define NTP_H

#include <Arduino.h>

bool connectWiFi();
void initNTP();
void waitForTime();

bool isWiFiConnected();
bool isAccessPointMode();

void saveWiFiCredentials(const String &ssid, const String &password);
void clearWiFiCredentials();

#endif