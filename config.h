#ifndef CONFIG_H
#define CONFIG_H

// DCF77 Ausgang
#define DCF_PIN 12

// NTP / Zeitzone - Werkseinstellungen
static const char* const NTP_SERVER_1 = "pool.ntp.org";
static const char* const NTP_SERVER_2 = "time.nist.gov";
static const char* const TIMEZONE_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

// Access Point für die WLAN-Konfiguration
static const char* const AP_SSID = "DCM77-Setup";

// Zeit für den WLAN-Verbindungsversuch
static const unsigned long WIFI_CONNECT_TIMEOUT_MS = 15000;

#endif

// Maximale Zeit seit der letzten erfolgreichen NTP-Synchronisation.
const unsigned long NTP_SYNC_TIMEOUT_SECONDS = 6UL * 60UL * 60UL;

// Prüfintervall des unabhängigen NTP-Watchdogs.
static const unsigned long NTP_WATCHDOG_INTERVAL_MS = 60000UL;
