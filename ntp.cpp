#include "ntp.h"
#include "config.h"

#include <WiFi.h>
#include <Preferences.h>
#include <time.h>

static Preferences preferences;

static bool wifiConnected = false;
static bool accessPointMode = false;


// ------------------------------------------------------------
// WLAN-Zugangsdaten aus NVS laden
// ------------------------------------------------------------

static bool loadWiFiCredentials(String &ssid, String &password)
{
    preferences.begin("wifi", true);

    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");

    preferences.end();

    return ssid.length() > 0;
}


// ------------------------------------------------------------
// WLAN-Zugangsdaten speichern
// ------------------------------------------------------------

void saveWiFiCredentials(const String &ssid, const String &password)
{
    preferences.begin("wifi", false);

    preferences.putString("ssid", ssid);
    preferences.putString("password", password);

    preferences.end();
}


// ------------------------------------------------------------
// WLAN-Zugangsdaten löschen
// ------------------------------------------------------------

void clearWiFiCredentials()
{
    preferences.begin("wifi", false);

    preferences.clear();

    preferences.end();
}


// ------------------------------------------------------------
// Access Point starten
// ------------------------------------------------------------

static void startAccessPoint()
{
    Serial.println();
    Serial.println("Keine WLAN-Verbindung möglich.");
    Serial.println("Starte Access Point...");

    WiFi.mode(WIFI_AP);

    if (WiFi.softAP(AP_SSID))
    {
        accessPointMode = true;
        wifiConnected = false;

        Serial.println();
        Serial.println("Access Point gestartet");
        Serial.print("SSID: ");
        Serial.println(AP_SSID);

        Serial.print("IP-Adresse: ");
        Serial.println(WiFi.softAPIP());
    }
    else
    {
        Serial.println("Fehler beim Start des Access Points!");
    }
}


// ------------------------------------------------------------
// WLAN verbinden
// ------------------------------------------------------------

bool connectWiFi()
{
    String ssid;
    String password;

    if (!loadWiFiCredentials(ssid, password))
    {
        Serial.println("Keine WLAN-Zugangsdaten gespeichert.");

        startAccessPoint();

        return false;
    }

    Serial.println();
    Serial.println("Gespeicherte WLAN-Konfiguration gefunden.");
    Serial.print("SSID: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());

    Serial.print("Verbinde mit WLAN");

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < WIFI_CONNECT_TIMEOUT_MS)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        accessPointMode = false;

        Serial.println("WLAN verbunden");

        Serial.print("IP-Adresse: ");
        Serial.println(WiFi.localIP());

        Serial.print("Signalstärke: ");
        Serial.print(WiFi.RSSI());
        Serial.println(" dBm");

        return true;
    }

    Serial.println("WLAN-Verbindung fehlgeschlagen.");

    WiFi.disconnect(true);

    startAccessPoint();

    return false;
}


// ------------------------------------------------------------
// NTP konfigurieren
// ------------------------------------------------------------

void initNTP()
{
    if (!wifiConnected)
    {
        Serial.println("NTP wird nicht gestartet - kein WLAN.");
        return;
    }

    configTime(
        0,
        0,
        NTP_SERVER_1,
        NTP_SERVER_2
    );

    setenv("TZ", TIMEZONE_INFO, 1);
    tzset();

    Serial.println("NTP konfiguriert.");
}


// ------------------------------------------------------------
// Auf gültige Zeit warten
// ------------------------------------------------------------

void waitForTime()
{
    struct tm timeinfo;

    while (!getLocalTime(&timeinfo))
    {
        Serial.println("Warte auf NTP...");
        delay(1000);
    }

    Serial.println("NTP-Zeit verfügbar.");
}


// ------------------------------------------------------------
// Status
// ------------------------------------------------------------

bool isWiFiConnected()
{
    return wifiConnected;
}


bool isAccessPointMode()
{
    return accessPointMode;
}