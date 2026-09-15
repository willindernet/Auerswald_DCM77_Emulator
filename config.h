#ifndef CONFIG_H
#define CONFIG_H

// --------------------------------------------------
// Hardware
// --------------------------------------------------

#define DCF_PIN 12


// --------------------------------------------------
// WLAN Konfiguration
// --------------------------------------------------

const char* ssid     = "MySSID";
const char* password = "MyPassword";


// --------------------------------------------------
// NTP
// --------------------------------------------------

static const char* const NTP_SERVER_1 = "pool.ntp.org";
static const char* const NTP_SERVER_2 = "time.nist.gov";


// --------------------------------------------------
// Zeitzone Deutschland
// Automatische Umschaltung Sommer-/Winterzeit
// --------------------------------------------------

static const char* const TIMEZONE_INFO =
    "CET-1CEST,M3.5.0,M10.5.0/3";

#endif