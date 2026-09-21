#include "ntp.h"
#include "config.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <time.h>
#include <sys/time.h>

static Preferences preferences;

static bool wifiConnected = false;
static bool accessPointMode = false;
static bool directTimeSynchronized = false;

// Time of the last successful direct NTP synchronization.
// millis() is sufficient for the 6-hour monitoring and is compared correctly
// using unsigned arithmetic, even in the event of an overflow.
static unsigned long lastSuccessfulNtpSync = 0;
static bool ntpSyncTimestampValid = false;



//---------------------------------------------------------------------------
// bool loadWiFiCredentials(String &ssid, String &password)
//---------------------------------------------------------------------------
// Description     | Load WIFI login credentials from NVS
// Parameter       | String ssid: WIFI ssid
// Parameter       | String password: WIFI password
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
static bool loadWiFiCredentials(String &ssid, String &password)
{
    preferences.begin("wifi", true);

    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");

    preferences.end();

    return ssid.length() > 0;
}


//---------------------------------------------------------------------------
// void saveWiFiCredentials(const String &ssid, const String &password)
//---------------------------------------------------------------------------
// Description     | Save WIFI login information
// Parameter       | String ssid: WIFI ssid
// Parameter       | String password: WIFI password
// Return value    | void
//---------------------------------------------------------------------------
void saveWiFiCredentials(const String &ssid, const String &password)
{
    preferences.begin("wifi", false);

    preferences.putString("ssid", ssid);
    preferences.putString("password", password);

    preferences.end();
}


//---------------------------------------------------------------------------
// void clearWiFiCredentials()
//---------------------------------------------------------------------------
// Description     | Delete WIFI login information
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void clearWiFiCredentials()
{
    preferences.begin("wifi", false);

    preferences.clear();

    preferences.end();
}


//---------------------------------------------------------------------------
// void startAccessPoint()
//---------------------------------------------------------------------------
// Description     | Start the access point
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
static void startAccessPoint()
{
    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println("Keine WLAN-Verbindung möglich.");
    if (DEBUG_SERIAL) Serial.println("Starte Access Point...");

    WiFi.mode(WIFI_AP);

    if (WiFi.softAP(AP_SSID))
    {
        accessPointMode = true;
        wifiConnected = false;

        if (DEBUG_SERIAL) Serial.println();
        if (DEBUG_SERIAL) Serial.println("Access Point gestartet");
        if (DEBUG_SERIAL) Serial.print("SSID: ");
        if (DEBUG_SERIAL) Serial.println(AP_SSID);

        if (DEBUG_SERIAL) Serial.print("IP-Adresse: ");
        if (DEBUG_SERIAL) Serial.println(WiFi.softAPIP());
    }
    else
    {
        if (DEBUG_SERIAL) Serial.println("Fehler beim Start des Access Points!");
    }
}


//---------------------------------------------------------------------------
// bool connectWiFi()
//---------------------------------------------------------------------------
// Description     | Connect to WIFI
// Parameter       | None
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
bool connectWiFi()
{
    // As a general rule, do not assume a valid NTP time at the start of every reboot.
    directTimeSynchronized = false;
    ntpSyncTimestampValid = false;
    lastSuccessfulNtpSync = 0;

    String ssid;
    String password;

    if (!loadWiFiCredentials(ssid, password))
    {
        if (DEBUG_SERIAL) Serial.println("Keine WLAN-Zugangsdaten gespeichert.");

        startAccessPoint();

        return false;
    }

    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println("Gespeicherte WLAN-Konfiguration gefunden.");
    if (DEBUG_SERIAL) Serial.print("SSID: ");
    if (DEBUG_SERIAL) Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(DEVICE_HOSTNAME);
    WiFi.begin(ssid.c_str(), password.c_str());

    if (DEBUG_SERIAL) Serial.print("Verbinde mit WLAN");

    unsigned long startTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startTime < WIFI_CONNECT_TIMEOUT * 1000UL)
    {
        delay(500);
        if (DEBUG_SERIAL) Serial.print(".");
    }

    if (DEBUG_SERIAL) Serial.println();

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected = true;
        accessPointMode = false;

        if (DEBUG_SERIAL) Serial.println("WLAN verbunden");

        if (MDNS.begin(DEVICE_HOSTNAME))
        {
            if (DEBUG_SERIAL) Serial.print("mDNS erreichbar unter: http://");
            if (DEBUG_SERIAL) Serial.print(DEVICE_HOSTNAME);
            if (DEBUG_SERIAL) Serial.println(".local");
        }
        else
        {
            if (DEBUG_SERIAL) Serial.println("mDNS konnte nicht gestartet werden.");
        }

        if (DEBUG_SERIAL) Serial.print("IP-Adresse: ");
        if (DEBUG_SERIAL) Serial.println(WiFi.localIP());

        if (DEBUG_SERIAL) Serial.print("Signalstärke: ");
        if (DEBUG_SERIAL) Serial.print(WiFi.RSSI());
        if (DEBUG_SERIAL) Serial.println(" dBm");

        return true;
    }

    if (DEBUG_SERIAL) Serial.println("WLAN-Verbindung fehlgeschlagen.");

    WiFi.disconnect(true);

    startAccessPoint();

    return false;
}


//---------------------------------------------------------------------------
// void loadTimeConfiguration()
//---------------------------------------------------------------------------
// Description     | Load time configuration from NVS
//                 | If no custom values have been saved yet,
//                 | the factory settings from config.h are returned.
// Parameter       | String server1: Address of NTP server
// Parameter       | String server2: Address of NTP server
// Parameter       | String timezone: time zone
// Return value    | void
//---------------------------------------------------------------------------
void loadTimeConfiguration(
    String &server1,
    String &server2,
    String &timezone)
{
    preferences.begin("timecfg", true);

    server1 = preferences.getString("ntp1", NTP_SERVER_1);
    server2 = preferences.getString("ntp2", NTP_SERVER_2);
    timezone = preferences.getString("tz", TIMEZONE_INFO);

    preferences.end();
}


//---------------------------------------------------------------------------
// void loadTimeConfiguration()
//---------------------------------------------------------------------------
// Description     | Save time configuration
// Parameter       | String server1: Address of NTP server
// Parameter       | String server2: Address of NTP server
// Parameter       | String timezone: time zone
// Return value    | void
//---------------------------------------------------------------------------
void saveTimeConfiguration(
    const String &server1,
    const String &server2,
    const String &timezone)
{
    preferences.begin("timecfg", false);

    preferences.putString("ntp1", server1);
    preferences.putString("ntp2", server2);
    preferences.putString("tz", timezone);

    preferences.end();
}


//---------------------------------------------------------------------------
// void resetTimeConfiguration()
//---------------------------------------------------------------------------
// Description     | Reset the time settings to factory defaults
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void resetTimeConfiguration()
{
    preferences.begin("timecfg", false);

    preferences.clear();

    preferences.end();
}


//---------------------------------------------------------------------------
// void getTimeConfiguration()
//---------------------------------------------------------------------------
// Description     | Provide the current time configuration
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void getTimeConfiguration(
    String &server1,
    String &server2,
    String &timezone)
{
    loadTimeConfiguration(server1, server2, timezone);
}


//---------------------------------------------------------------------------
// void ntpTimeSyncCallback(struct timeval *tv)
//---------------------------------------------------------------------------
// Description     | NTP Diagnostics
// Parameter       | struct timeval *tv: actual time
// Return value    | void
//---------------------------------------------------------------------------
static void ntpTimeSyncCallback(struct timeval *tv)
{
    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println(">>> NTP-Synchronisation erfolgreich <<<");

//---------------------------------------------------------------------------
    if (tv != nullptr)
    {
        if (DEBUG_SERIAL) Serial.print("Unix-Zeit: ");
        if (DEBUG_SERIAL) Serial.println((long)tv->tv_sec);
    }

    time_t now = time(nullptr);
    struct tm localTime;

    localtime_r(&now, &localTime);

    if (DEBUG_SERIAL) Serial.printf(
        "Lokale Zeit: %04d-%02d-%02d %02d:%02d:%02d\n",
        localTime.tm_year + 1900,
        localTime.tm_mon + 1,
        localTime.tm_mday,
        localTime.tm_hour,
        localTime.tm_min,
        localTime.tm_sec
    );
}


//---------------------------------------------------------------------------
// void testNtpDns(const String &serverName)
//---------------------------------------------------------------------------
// Description     | Check whether the NTP server is reachable
// Parameter       | String serverName: Address of NTP server
// Return value    | void
//---------------------------------------------------------------------------
static void testNtpDns(const String &serverName)
{
    if (serverName.length() == 0)
    {
        if (DEBUG_SERIAL) Serial.println("DNS-Test: Servername ist leer.");
        return;
    }

    IPAddress resolved;

    if (DEBUG_SERIAL) Serial.print("DNS-Test fuer ");
    if (DEBUG_SERIAL) Serial.print(serverName);
    if (DEBUG_SERIAL) Serial.print(": ");

    int result = WiFi.hostByName(serverName.c_str(), resolved);

    if (result == 1)
    {
        if (DEBUG_SERIAL) Serial.print("OK -> ");
        if (DEBUG_SERIAL) Serial.println(resolved);
    }
    else
    {
        if (DEBUG_SERIAL) Serial.println("FEHLER - Name konnte nicht aufgeloest werden.");
    }
}


//---------------------------------------------------------------------------
// bool setSystemTimeFromNtpPacket(const uint8_t *packet, unsigned long roundTripMs)
//---------------------------------------------------------------------------
// Description     | set system time from NTP packet
// Parameter       | uint8_t packet: NTP packet
// Parameter       | unsigned long roundTripMs: round trip in ms
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
static bool setSystemTimeFromNtpPacket(const uint8_t *packet, unsigned long roundTripMs)
{
    if (packet == nullptr)
    {
        return false;
    }

    // NTP transmit timestamp: bytes 40..47.
    uint32_t seconds =
        ((uint32_t)packet[40] << 24) |
        ((uint32_t)packet[41] << 16) |
        ((uint32_t)packet[42] << 8) |
        ((uint32_t)packet[43]);

    uint32_t fraction =
        ((uint32_t)packet[44] << 24) |
        ((uint32_t)packet[45] << 16) |
        ((uint32_t)packet[46] << 8) |
        ((uint32_t)packet[47]);

    const uint32_t NTP_UNIX_OFFSET = 2208988800UL;

    if (seconds < NTP_UNIX_OFFSET)
    {
        return false;
    }

    uint64_t unixSeconds = (uint64_t)seconds - NTP_UNIX_OFFSET;

    // The server transmit timestamp is taken just before the response leaves
    // the server. Add approximately half the measured round-trip time so that
    // the local clock is set close to the instant at which the response was
    // received. This is sufficient for the minute-based DCF77 generation.
    uint64_t micros = ((uint64_t)fraction * 1000000ULL) >> 32;
    micros += ((uint64_t)roundTripMs * 1000ULL) / 2ULL;

    unixSeconds += micros / 1000000ULL;
    micros %= 1000000ULL;

    struct timeval tv;
    tv.tv_sec = (time_t)unixSeconds;
    tv.tv_usec = (suseconds_t)micros;

    if (settimeofday(&tv, nullptr) != 0)
    {
        if (DEBUG_SERIAL) Serial.println("Fehler: settimeofday() konnte die Systemzeit nicht setzen.");
        return false;
    }

    directTimeSynchronized = true;
    lastSuccessfulNtpSync = millis();
    ntpSyncTimestampValid = true;
    return true;
}


//---------------------------------------------------------------------------
// bool testNtpUdpServer(const String &serverName, bool setSystemTime)
//---------------------------------------------------------------------------
// Description     | test NTP UDP server
// Parameter       | String serverName: Name of NTP server
// Parameter       | bool setSystemTime: set system time 
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
static bool testNtpUdpServer(const String &serverName, bool setSystemTime)
{
    if (serverName.length() == 0)
    {
        if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: Servername ist leer.");
        return false;
    }

    IPAddress serverIP;

    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println("----------------------------------------");
    if (DEBUG_SERIAL) Serial.print("Direkter UDP-NTP-Test: ");
    if (DEBUG_SERIAL) Serial.println(serverName);

    if (WiFi.hostByName(serverName.c_str(), serverIP) != 1)
    {
        if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: DNS-Aufloesung fehlgeschlagen.");
        if (DEBUG_SERIAL) Serial.println("----------------------------------------");
        return false;
    }

    if (DEBUG_SERIAL) Serial.print("NTP-Server IP: ");
    if (DEBUG_SERIAL) Serial.println(serverIP);
    if (DEBUG_SERIAL) Serial.println("Sende NTP-Anfrage an UDP Port 123...");

    WiFiUDP udp;
    const uint16_t localPort = 2390;

    if (!udp.begin(localPort))
    {
        if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: Lokaler UDP-Port konnte nicht geoeffnet werden.");
        if (DEBUG_SERIAL) Serial.println("----------------------------------------");
        return false;
    }

    uint8_t packet[48] = {0};
    packet[0] = 0x1B; // NTP Version 3, Client Mode.

    unsigned long sendMillis = millis();

    if (!udp.beginPacket(serverIP, 123))
    {
        if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: beginPacket() fehlgeschlagen.");
        udp.stop();
        if (DEBUG_SERIAL) Serial.println("----------------------------------------");
        return false;
    }

    udp.write(packet, sizeof(packet));

    if (!udp.endPacket())
    {
        if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: endPacket() fehlgeschlagen.");
        udp.stop();
        if (DEBUG_SERIAL) Serial.println("----------------------------------------");
        return false;
    }

    if (DEBUG_SERIAL) Serial.println("NTP-Anfrage gesendet. Warte bis zu 3 Sekunden auf Antwort...");

    unsigned long start = millis();

    while (millis() - start < 3000)
    {
        int packetSize = udp.parsePacket();

        if (packetSize > 0)
        {
            unsigned long receiveMillis = millis();
            unsigned long roundTripMs = receiveMillis - sendMillis;

            if (DEBUG_SERIAL) Serial.print("UDP-NTP-Antwort empfangen, Bytes: ");
            if (DEBUG_SERIAL) Serial.println(packetSize);

            if (packetSize < 48)
            {
                if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: Antwort ist zu kurz fuer ein NTP-Paket.");
                udp.stop();
                if (DEBUG_SERIAL) Serial.println("----------------------------------------");
                return false;
            }

            int bytesRead = udp.read(packet, sizeof(packet));

            if (bytesRead < 48)
            {
                if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: NTP-Paket konnte nicht vollstaendig gelesen werden.");
                udp.stop();
                if (DEBUG_SERIAL) Serial.println("----------------------------------------");
                return false;
            }

            uint8_t li = (packet[0] >> 6) & 0x03;
            uint8_t version = (packet[0] >> 3) & 0x07;
            uint8_t mode = packet[0] & 0x07;
            uint8_t stratum = packet[1];

            if (DEBUG_SERIAL) Serial.print("NTP Antwort: LI=");
            if (DEBUG_SERIAL) Serial.print(li);
            if (DEBUG_SERIAL) Serial.print(", Version=");
            if (DEBUG_SERIAL) Serial.print(version);
            if (DEBUG_SERIAL) Serial.print(", Mode=");
            if (DEBUG_SERIAL) Serial.print(mode);
            if (DEBUG_SERIAL) Serial.print(", Stratum=");
            if (DEBUG_SERIAL) Serial.println(stratum);

            uint32_t seconds =
                ((uint32_t)packet[40] << 24) |
                ((uint32_t)packet[41] << 16) |
                ((uint32_t)packet[42] << 8) |
                ((uint32_t)packet[43]);

            uint32_t fraction =
                ((uint32_t)packet[44] << 24) |
                ((uint32_t)packet[45] << 16) |
                ((uint32_t)packet[46] << 8) |
                ((uint32_t)packet[47]);

            const uint32_t NTP_UNIX_OFFSET = 2208988800UL;

            if (seconds >= NTP_UNIX_OFFSET)
            {
                time_t unixTime = (time_t)(seconds - NTP_UNIX_OFFSET);
                struct tm utcTime;
                gmtime_r(&unixTime, &utcTime);

                if (DEBUG_SERIAL) Serial.printf(
                    "NTP-Zeitstempel: %04d-%02d-%02d %02d:%02d:%02d UTC\n",
                    utcTime.tm_year + 1900,
                    utcTime.tm_mon + 1,
                    utcTime.tm_mday,
                    utcTime.tm_hour,
                    utcTime.tm_min,
                    utcTime.tm_sec
                );

                if (DEBUG_SERIAL) Serial.print("NTP-Fraktion: 0x");
                if (DEBUG_SERIAL) Serial.printf("%08lX\n", (unsigned long)fraction);
                if (DEBUG_SERIAL) Serial.print("Round Trip Time: ");
                if (DEBUG_SERIAL) Serial.print(roundTripMs);
                if (DEBUG_SERIAL) Serial.println(" ms");
            }
            else
            {
                if (DEBUG_SERIAL) Serial.println("NTP-Zeitstempel ist ungueltig.");
            }

            if (mode == 4 && stratum >= 1 && stratum <= 15 && seconds >= NTP_UNIX_OFFSET)
            {
                if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: *** NTP-ANTWORT IST GÜLTIG ***");
                if (DEBUG_SERIAL) Serial.println("UDP/123 ist aus dem WLAN erreichbar.");

                if (setSystemTime)
                {
                    if (setSystemTimeFromNtpPacket(packet, roundTripMs))
                    {
                        if (DEBUG_SERIAL) Serial.println("Systemzeit wurde direkt aus der NTP-Antwort gesetzt.");
                    }
                    else
                    {
                        if (DEBUG_SERIAL) Serial.println("Systemzeit konnte NICHT gesetzt werden.");
                        udp.stop();
                        if (DEBUG_SERIAL) Serial.println("----------------------------------------");
                        return false;
                    }
                }

                udp.stop();
                if (DEBUG_SERIAL) Serial.println("----------------------------------------");
                return true;
            }

            if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: Antwort empfangen, aber nicht als gueltige Serverantwort erkannt.");
            udp.stop();
            if (DEBUG_SERIAL) Serial.println("----------------------------------------");
            return false;
        }

        delay(10);
    }

    if (DEBUG_SERIAL) Serial.println("UDP-NTP-Test: *** KEINE ANTWORT INNERHALB VON 3 SEKUNDEN ***");
    if (DEBUG_SERIAL) Serial.println("DNS funktioniert, aber UDP/123 hat keine NTP-Antwort geliefert.");
    udp.stop();
    if (DEBUG_SERIAL) Serial.println("----------------------------------------");
    return false;
}


//---------------------------------------------------------------------------
// bool runDirectNtpUdpDiagnostics(const String &server1, const String &server2)
//---------------------------------------------------------------------------
// Description     | run direct NTP / UDP diagnostics
// Parameter       | String server1: server name
// Parameter       | String server2: server name
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
static bool runDirectNtpUdpDiagnostics(const String &server1, const String &server2)
{
    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println("========================================");
    if (DEBUG_SERIAL) Serial.println("Direkter UDP-NTP-Zeitsynchronisationstest");
    if (DEBUG_SERIAL) Serial.println("========================================");
    if (DEBUG_SERIAL) Serial.println("Die Systemzeit wird direkt aus der NTP-Antwort gesetzt.");
    if (DEBUG_SERIAL) Serial.println("Die ESP32-SNTP-Automatik wird fuer v10 nicht benoetigt.");

    // Server 1 synchronizes the clock. Server 2 serves as a fallback.
    bool server1Ok = testNtpUdpServer(server1, true);
    bool server2Ok = false;

    if (!server1Ok)
    {
        if (DEBUG_SERIAL) Serial.println();
        if (DEBUG_SERIAL) Serial.println("Server 1 konnte die Systemzeit nicht setzen.");
        if (DEBUG_SERIAL) Serial.println("Versuche Server 2 als NTP-Fallback...");
        server2Ok = testNtpUdpServer(server2, true);
    }

    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println("===== Ergebnis direkter NTP-Synchronisation =====");
    if (DEBUG_SERIAL) Serial.print("Server 1: ");
    if (DEBUG_SERIAL) Serial.println(server1Ok ? "ZEIT GESETZT" : "nicht verwendet/fehlgeschlagen");
    if (DEBUG_SERIAL) Serial.print("Server 2: ");
    if (DEBUG_SERIAL) Serial.println(server2Ok ? "ZEIT GESETZT" : "nicht verwendet/kein Fallback benoetigt");

    if (directTimeSynchronized)
    {
        if (DEBUG_SERIAL) Serial.println("*** DIREKTE NTP-SYNCHRONISATION ERFOLGREICH ***");
        if (DEBUG_SERIAL) Serial.println("Die ESP32-Systemzeit ist jetzt gesetzt.");
        if (DEBUG_SERIAL) Serial.println("================================================");
        return true;
    }

    if (DEBUG_SERIAL) Serial.println("*** NTP-SYNCHRONISATION FEHLGESCHLAGEN ***");
    if (DEBUG_SERIAL) Serial.println("Kein NTP-Server konnte die Systemzeit setzen.");
    if (DEBUG_SERIAL) Serial.println("================================================");
    return false;
}


//---------------------------------------------------------------------------
// void printNetworkDiagnostics()
//---------------------------------------------------------------------------
// Description     | print network diagnostics
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
static void printNetworkDiagnostics()
{
    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println("===== Netzwerk-Diagnose =====");

    if (DEBUG_SERIAL) Serial.print("WiFi.status(): ");
    if (DEBUG_SERIAL) Serial.println(WiFi.status());

    if (DEBUG_SERIAL) Serial.print("SSID: ");
    if (DEBUG_SERIAL) Serial.println(WiFi.SSID());

    if (DEBUG_SERIAL) Serial.print("IP-Adresse: ");
    if (DEBUG_SERIAL) Serial.println(WiFi.localIP());

    if (DEBUG_SERIAL) Serial.print("Gateway: ");
    if (DEBUG_SERIAL) Serial.println(WiFi.gatewayIP());

    if (DEBUG_SERIAL) Serial.print("Subnetzmaske: ");
    if (DEBUG_SERIAL) Serial.println(WiFi.subnetMask());

    if (DEBUG_SERIAL) Serial.print("DNS 1: ");
    if (DEBUG_SERIAL) Serial.println(WiFi.dnsIP(0));

    if (DEBUG_SERIAL) Serial.print("DNS 2: ");
    if (DEBUG_SERIAL) Serial.println(WiFi.dnsIP(1));

    if (DEBUG_SERIAL) Serial.print("RSSI: ");
    if (DEBUG_SERIAL) Serial.print(WiFi.RSSI());
    if (DEBUG_SERIAL) Serial.println(" dBm");

    if (DEBUG_SERIAL) Serial.println("=============================");
    if (DEBUG_SERIAL) Serial.println();
}


//---------------------------------------------------------------------------
// bool isTimeValid()
//---------------------------------------------------------------------------
// Description     | Validity of the time source
// Parameter       | None
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
// Important: getLocalTime() alone is not sufficient here. The ESP32 system clock
// may still contain an old or retained time after a reboot. For
// DCF77 output, therefore, only a successful NTP synchronization
// since the current startup counts.

bool isTimeValid()
{
    return directTimeSynchronized;
}


//---------------------------------------------------------------------------
// void initNTP()
//---------------------------------------------------------------------------
// Description     | Configure NTP
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void initNTP()
{
    if (!wifiConnected)
    {
        if (DEBUG_SERIAL) Serial.println("NTP wird nicht gestartet - kein WLAN.");
        return;
    }

    String server1;
    String server2;
    String timezone;

    loadTimeConfiguration(server1, server2, timezone);

    server1.trim();
    server2.trim();
    timezone.trim();

    directTimeSynchronized = false;

    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println("========================================");
    if (DEBUG_SERIAL) Serial.println("NTP-Konfiguration v10");
    if (DEBUG_SERIAL) Serial.println("========================================");

    if (DEBUG_SERIAL) Serial.print("Server 1: ");
    if (DEBUG_SERIAL) Serial.println(server1);

    if (DEBUG_SERIAL) Serial.print("Server 2: ");
    if (DEBUG_SERIAL) Serial.println(server2);

    if (DEBUG_SERIAL) Serial.print("Zeitzone: ");
    if (DEBUG_SERIAL) Serial.println(timezone);

    printNetworkDiagnostics();

    if (DEBUG_SERIAL) Serial.println("Teste DNS-Aufloesung der konfigurierten NTP-Server:");
    testNtpDns(server1);
    testNtpDns(server2);

    bool synchronized = runDirectNtpUdpDiagnostics(server1, server2);

    setenv("TZ", timezone.c_str(), 1);
    tzset();

    if (synchronized)
    {
        if (DEBUG_SERIAL) Serial.println();
        if (DEBUG_SERIAL) Serial.println("Zeitzone gesetzt:");
        if (DEBUG_SERIAL) Serial.println(timezone);

        time_t now = time(nullptr);
        struct tm localTime;
        localtime_r(&now, &localTime);

        if (DEBUG_SERIAL) Serial.printf(
            "Aktuelle lokale Zeit: %04d-%02d-%02d %02d:%02d:%02d\n",
            localTime.tm_year + 1900,
            localTime.tm_mon + 1,
            localTime.tm_mday,
            localTime.tm_hour,
            localTime.tm_min,
            localTime.tm_sec
        );

        if (DEBUG_SERIAL) Serial.println("NTP-Zeitbasis v10 ist bereit.");
    }
    else
    {
        if (DEBUG_SERIAL) Serial.println("NTP-Zeitbasis v10 konnte nicht initialisiert werden.");
    }
}


//---------------------------------------------------------------------------
// void waitForTime()
//---------------------------------------------------------------------------
// Description     | Wait for the valid time
// Parameter       | None
// Return value    | void
//---------------------------------------------------------------------------
void waitForTime()
{
    struct tm timeinfo;

    if (DEBUG_SERIAL) Serial.println();
    if (DEBUG_SERIAL) Serial.println("Pruefe NTP-Zeitbasis...");

    if (!directTimeSynchronized)
    {
        if (DEBUG_SERIAL) Serial.println("Keine direkte NTP-Synchronisation vorhanden.");
        if (DEBUG_SERIAL) Serial.println("DCF77-Ausgabe wird nicht gestartet.");
        return;
    }

    if (getLocalTime(&timeinfo, 2000))
    {
        if (DEBUG_SERIAL) Serial.println();
        if (DEBUG_SERIAL) Serial.println("========================================");
        if (DEBUG_SERIAL) Serial.println("NTP-Zeit verfügbar.");
        if (DEBUG_SERIAL) Serial.printf(
            "Lokale Zeit: %04d-%02d-%02d %02d:%02d:%02d\n",
            timeinfo.tm_year + 1900,
            timeinfo.tm_mon + 1,
            timeinfo.tm_mday,
            timeinfo.tm_hour,
            timeinfo.tm_min,
            timeinfo.tm_sec
        );
        if (DEBUG_SERIAL) Serial.print("Unix-Zeit: ");
        if (DEBUG_SERIAL) Serial.println((long)time(nullptr));
        if (DEBUG_SERIAL) Serial.println("========================================");
        return;
    }

    if (DEBUG_SERIAL) Serial.println("Die Systemzeit wurde gesetzt, aber getLocalTime() liefert keine gueltige Zeit.");
    if (DEBUG_SERIAL) Serial.print("time(nullptr): ");
    if (DEBUG_SERIAL) Serial.println((long)time(nullptr));
}


//---------------------------------------------------------------------------
// bool isWiFiConnected()
//---------------------------------------------------------------------------
// Description     | WIFI conection status
// Parameter       | None
// Return value    | bool: 1 -> connected
//                 | bool: 0 -> not connected
//---------------------------------------------------------------------------
bool isWiFiConnected()
{
    return wifiConnected;
}


//---------------------------------------------------------------------------
// bool isAccessPointMode()
//---------------------------------------------------------------------------
// Description     | check access point mode
// Parameter       | None
// Return value    | bool: 1 -> access point mode
//                 | bool: 0 -> not access point mode
//---------------------------------------------------------------------------
bool isAccessPointMode()
{
    return accessPointMode;
}

//---------------------------------------------------------------------------
// bool hasWiFiCredentials()
//---------------------------------------------------------------------------
// Description     | check for WIFI credentials available
// Parameter       | None
// Return value    | bool: 1 -> credentials available
//                 | bool: 0 -> no credentials available
//---------------------------------------------------------------------------
bool hasWiFiCredentials()
{
    String ssid;
    String password;
    return loadWiFiCredentials(ssid, password);
}


//---------------------------------------------------------------------------
// bool isNtpSynchronizationFresh()
//---------------------------------------------------------------------------
// Description     | NTP synchronization status for the watchdog
// Parameter       | None
// Return value    | bool: 1 -> last NTP synchronisation in configured time window
//                 | bool: 0 -> no NTP synchronisation in configured time window
//---------------------------------------------------------------------------
bool isNtpSynchronizationFresh()
{
    if (!directTimeSynchronized || !ntpSyncTimestampValid)
        return false;

    uint32_t elapsedSeconds =
        (uint32_t)((millis() - lastSuccessfulNtpSync) / 1000UL);

    return elapsedSeconds < NTP_SYNC_TIMEOUT * 60UL * 60UL;
}

//---------------------------------------------------------------------------
// bool refreshNtpSynchronization()
//---------------------------------------------------------------------------
// Description     | refresh NTP synchronization
// Parameter       | None
// Return value    | bool: 1 -> OK
//                 | bool: 0 -> NOK
//---------------------------------------------------------------------------
bool refreshNtpSynchronization()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        directTimeSynchronized = false;
        return false;
    }

    String server1;
    String server2;
    String timezone;

    loadTimeConfiguration(server1, server2, timezone);

    server1.trim();
    server2.trim();
    timezone.trim();

    if (timezone.length() > 0)
    {
        setenv("TZ", timezone.c_str(), 1);
        tzset();
    }

    if (testNtpUdpServer(server1, true))
        return true;

    if (testNtpUdpServer(server2, true))
        return true;

    return false;
}
