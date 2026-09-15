#include "ntp.h"
#include "config.h"

#include <WiFi.h>
#include <time.h>


void connectWiFi()
{
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WLAN verbunden");
}


void initNTP()
{
    // NTP konfigurieren
    configTime(
        0,
        0,
        NTP_SERVER_1,
        NTP_SERVER_2
    );

    // Zeitzone setzen
    // Deutschland, automatische Umschaltung
    // Sommer-/Winterzeit
    setenv("TZ", TIMEZONE_INFO, 1);
    tzset();
}


void waitForTime()
{
    // Warten, bis Zeitinformation per NTP empfangen wurde
    struct tm timeinfo;

    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Warte auf NTP...");
        delay(1000);
    }
}