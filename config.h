#ifndef CONFIG_H
#define CONFIG_H

// ============================================================
// Start of configuration
// ============================================================

// seriell debuging
#define DEBUG_SERIAL 0

// DCF77 output
#define DCF_PIN 12

// NTP / Time Zone - Factory Settings
static const char* const NTP_SERVER_1 = "pool.ntp.org";
static const char* const NTP_SERVER_2 = "time.nist.gov";
static const char* const TIMEZONE_INFO = "CET-1CEST,M3.5.0,M10.5.0/3";

// Access Point for WIFI-Configuration
static const char* const AP_SSID = "DCM77-Setup";

// Network hostname and mDNS name
static const char* const DEVICE_HOSTNAME = "DCM77";

// Time to try connecting to Wi-Fi
static const unsigned long WIFI_CONNECT_TIMEOUT = 15;       // seconds

// Maximum time since the last successful NTP synchronization
static const unsigned long NTP_SYNC_TIMEOUT = 6;            // hours

// Check interval of the NTP watchdog
static const unsigned long NTP_WATCHDOG_INTERVAL = 1;       // minutes

// ============================================================
// End of configuration
// ============================================================

#endif
