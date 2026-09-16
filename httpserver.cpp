#include "httpserver.h"
#include "ntp.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

static WebServer httpServer(80);
static TaskHandle_t httpServerTaskHandle = nullptr;


// ------------------------------------------------------------
// HTML-Hilfsfunktion: HTML-Zeichen escapen
// ------------------------------------------------------------

static String htmlEscape(const String &value)
{
    String result = value;

    result.replace("&", "&amp;");
    result.replace("<", "&lt;");
    result.replace(">", "&gt;");
    result.replace("\"", "&quot;");

    return result;
}


// ------------------------------------------------------------
// Zeitkonfiguration: bekannte Zeitzonen
//
// Die Auswahl speichert die jeweilige POSIX-TZ-Zeichenkette.
// "Benutzerdefiniert" erlaubt die direkte Eingabe einer
// beliebigen POSIX-TZ-Zeichenkette.
// ------------------------------------------------------------

struct TimezoneOption
{
    const char* name;
    const char* value;
};

static const TimezoneOption timezoneOptions[] =
{
    // UTC / Europa
    { "UTC", "UTC0" },
    { "Deutschland / Mitteleuropa", "CET-1CEST,M3.5.0,M10.5.0/3" },
    { "Großbritannien / Irland / Portugal", "GMT0BST,M3.5.0/1,M10.5.0" },
    { "Island", "GMT0" },
    { "Azoren", "AZOT1AZOST,M3.5.0/0,M10.5.0/1" },
    { "Osteuropa / Griechenland / Finnland", "EET-2EEST,M3.5.0/3,M10.5.0/4" },
    { "Türkei", "TRT-3" },
    { "Moskau", "MSK-3" },
    { "Minsk", "MSK-3" },

    // Nordamerika
    { "US Eastern (New York / Toronto)", "EST5EDT,M3.2.0,M11.1.0" },
    { "US Central (Chicago / Winnipeg)", "CST6CDT,M3.2.0,M11.1.0" },
    { "US Mountain (Denver / Calgary)", "MST7MDT,M3.2.0,M11.1.0" },
    { "US Pacific (Los Angeles / Vancouver)", "PST8PDT,M3.2.0,M11.1.0" },
    { "Arizona", "MST7" },
    { "Alaska", "AKST9AKDT,M3.2.0,M11.1.0" },
    { "Hawaii", "HST10" },
    { "Atlantic (Halifax)", "AST4ADT,M3.2.0,M11.1.0" },
    { "Newfoundland", "NST3:30NDT,M3.2.0,M11.1.0" },

    // Mittel- und Südamerika
    { "Brasilien / Sao Paulo", "BRT3" },
    { "Argentinien / Buenos Aires", "ART3" },
    { "Chile / Santiago", "CLT4CLST,M9.1.0/0,M4.1.0/0" },

    // Nahost / Afrika
    { "Arabische Golfstaaten (Dubai)", "GST-4" },
    { "Saudi-Arabien (Riad)", "AST-3" },
    { "Südafrika (Johannesburg)", "SAST-2" },
    { "Westafrika (Lagos)", "WAT-1" },
    { "Ägypten (Kairo)", "EET-2" },

    // Asien
    { "Israel", "IST-2IDT,M3.4.4/26,M10.5.0" },
    { "Iran (Teheran)", "IRST-3:30" },
    { "Indien (Kolkata)", "IST-5:30" },
    { "Nepal (Kathmandu)", "NPT-5:45" },
    { "Bangkok / Jakarta", "WIB-7" },
    { "China / Hongkong / Singapur", "CST-8" },
    { "Japan", "JST-9" },
    { "Korea", "KST-9" },
    { "Taiwan", "CST-8" },

    // Australien / Neuseeland
    { "Australien West (Perth)", "AWST-8" },
    { "Australien Zentral (Darwin)", "ACST-9:30" },
    { "Australien Ost (Brisbane)", "AEST-10" },
    { "Australien Südost (Sydney / Melbourne)", "AEST-10AEDT,M10.1.0,M4.1.0/3" },
    { "Neuseeland", "NZST-12NZDT,M9.5.0,M4.1.0/3" }
};

static const size_t timezoneOptionCount =
    sizeof(timezoneOptions) / sizeof(timezoneOptions[0]);


// ------------------------------------------------------------
// Zeitzonen-Auswahl erzeugen
// ------------------------------------------------------------

static String makeTimezoneOptions(const String &currentTimezone)
{
    String html;

    bool found = false;

    for (size_t i = 0; i < timezoneOptionCount; i++)
    {
        html += "<option value='";
        html += htmlEscape(timezoneOptions[i].value);
        html += "'";

        if (currentTimezone == timezoneOptions[i].value)
        {
            html += " selected";
            found = true;
        }

        html += ">";
        html += timezoneOptions[i].name;
        html += "</option>";
    }

    html += "<option value='custom'";

    if (!found)
    {
        html += " selected";
    }

    html += ">Benutzerdefiniert</option>";

    return html;
}


// ------------------------------------------------------------
// HTML-Seite
// ------------------------------------------------------------

static String makePage()
{
    String server1;
    String server2;
    String timezone;

    getTimeConfiguration(server1, server2, timezone);

    String html;

    html += "<!DOCTYPE html>";
    html += "<html lang='de'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>DCM-77 Emulator</title>";

    html += "<style>";
    html += "body{font-family:Arial,sans-serif;max-width:700px;margin:40px auto;padding:20px;}";
    html += "h1{font-size:24px;}";
    html += "h2{margin-top:30px;}";
    html += "label{display:block;margin-top:15px;}";
    html += "input,select{width:100%;padding:10px;margin-top:5px;box-sizing:border-box;}";
    html += "button{margin-top:20px;padding:12px 20px;font-size:16px;}";
    html += ".status{padding:15px;background:#eee;margin:20px 0;}";
    html += ".section{border-top:1px solid #ccc;margin-top:30px;padding-top:10px;}";
    html += ".hint{font-size:14px;color:#555;}";
    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<h1>Auerswald DCM-77 Emulator</h1>";

    html += "<div class='status'>";

    if (isAccessPointMode())
    {
        html += "<b>WLAN-Modus:</b> Access Point<br>";
        html += "<b>SSID:</b> ";
        html += htmlEscape(AP_SSID);
        html += "<br>";
        html += "<b>IP-Adresse:</b> ";
        html += WiFi.softAPIP().toString();
    }
    else if (isWiFiConnected())
    {
        html += "<b>WLAN-Modus:</b> verbunden<br>";
        html += "<b>SSID:</b> ";
        html += htmlEscape(WiFi.SSID());
        html += "<br>";
        html += "<b>IP-Adresse:</b> ";
        html += WiFi.localIP().toString();
        html += "<br>";
        html += "<b>Signalstärke:</b> ";
        html += String(WiFi.RSSI());
        html += " dBm";
    }
    else
    {
        html += "<b>WLAN-Modus:</b> nicht verbunden";
    }

    html += "</div>";

    // --------------------------------------------------------
    // WLAN
    // --------------------------------------------------------

    html += "<h2>WLAN konfigurieren</h2>";

    html += "<form action='/save' method='POST'>";

    html += "<label>SSID</label>";
    html += "<input type='text' name='ssid' required>";

    html += "<label>Passwort</label>";
    html += "<input type='password' name='password' required>";

    html += "<button type='submit'>WLAN speichern und Neustart</button>";

    html += "</form>";

    html += "<form action='/clear' method='POST'>";
    html += "<button type='submit'>WLAN-Konfiguration löschen</button>";
    html += "</form>";


    // --------------------------------------------------------
    // NTP / Zeitzone
    // --------------------------------------------------------

    html += "<div class='section'>";
    html += "<h2>NTP und Zeitzone</h2>";

    html += "<form action='/timesave' method='POST'>";

    html += "<label>NTP Server 1</label>";
    html += "<input type='text' name='ntp1' value='";
    html += htmlEscape(server1);
    html += "' required>";

    html += "<label>NTP Server 2</label>";
    html += "<input type='text' name='ntp2' value='";
    html += htmlEscape(server2);
    html += "' required>";

    html += "<label>Zeitzone</label>";
    html += "<select name='timezone' onchange='showCustomTimezone(this.value)'>";
    html += makeTimezoneOptions(timezone);
    html += "</select>";

    html += "<div id='customTimezoneDiv' style='display:none'>";

    html += "<label>Benutzerdefinierte Zeitzone</label>";
    html += "<input type='text' id='customTimezone' name='customTimezone' value='";
    html += htmlEscape(timezone);
    html += "'>";

    html += "<p class='hint'>";
    html += "POSIX-TZ-Zeichenkette, z. B. ";
    html += "CET-1CEST,M3.5.0,M10.5.0/3";
    html += "</p>";

    html += "</div>";

    html += "<button type='submit'>NTP / Zeitzone speichern und Neustart</button>";

    html += "</form>";

    html += "<form action='/timereset' method='POST'>";
    html += "<button type='submit'>Werkseinstellungen für NTP / Zeitzone laden</button>";
    html += "</form>";

    html += "</div>";

    // --------------------------------------------------------
    // JavaScript
    // --------------------------------------------------------

    html += "<script>";
    html += "function showCustomTimezone(value){";
    html += "document.getElementById('customTimezoneDiv').style.display=";
    html += "(value==='custom')?'block':'none';";
    html += "}";
    html += "showCustomTimezone(document.querySelector('select[name=timezone]').value);";
    html += "</script>";

    html += "</body>";
    html += "</html>";

    return html;
}


// ------------------------------------------------------------
// Startseite
// ------------------------------------------------------------

static void handleRoot()
{
    httpServer.send(
        200,
        "text/html; charset=utf-8",
        makePage()
    );
}


// ------------------------------------------------------------
// WLAN speichern
// ------------------------------------------------------------

static void handleSave()
{
    if (!httpServer.hasArg("ssid") || !httpServer.hasArg("password"))
    {
        httpServer.send(
            400,
            "text/plain; charset=utf-8",
            "SSID und Passwort muessen eingegeben werden."
        );

        return;
    }

    String ssid = httpServer.arg("ssid");
    String password = httpServer.arg("password");

    ssid.trim();
    password.trim();

    if (ssid.length() == 0 || password.length() == 0)
    {
        httpServer.send(
            400,
            "text/html; charset=utf-8",
            "<!DOCTYPE html>"
            "<html lang='de'>"
            "<head>"
            "<meta charset='UTF-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "<title>DCM-77 Emulator</title>"
            "</head>"
            "<body>"
            "<h1>Fehler</h1>"
            "<p>SSID und Passwort duerfen nicht leer sein.</p>"
            "<p><a href='/'>Zurueck zur WLAN-Konfiguration</a></p>"
            "</body>"
            "</html>"
        );

        return;
    }

    saveWiFiCredentials(ssid, password);

    httpServer.send(
        200,
        "text/html; charset=utf-8",
        "<!DOCTYPE html>"
        "<html lang='de'>"
        "<head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>DCM-77 Emulator</title>"
        "</head>"
        "<body>"
        "<h1>WLAN gespeichert</h1>"
        "<p>Die neuen WLAN-Zugangsdaten wurden gespeichert.</p>"
        "<p>Der ESP32 startet jetzt neu.</p>"
        "</body>"
        "</html>"
    );

    delay(1000);

    ESP.restart();
}


// ------------------------------------------------------------
// WLAN-Konfiguration löschen
// ------------------------------------------------------------

static void handleClear()
{
    clearWiFiCredentials();

    httpServer.send(
        200,
        "text/html; charset=utf-8",
        "<!DOCTYPE html>"
        "<html lang='de'>"
        "<head><meta charset='UTF-8'></head>"
        "<body>"
        "<h1>WLAN-Konfiguration gelöscht</h1>"
        "<p>Der ESP32 startet neu und öffnet anschließend den "
        "Setup-Access-Point.</p>"
        "</body>"
        "</html>"
    );

    delay(1000);

    ESP.restart();
}


// ------------------------------------------------------------
// NTP / Zeitzone speichern
// ------------------------------------------------------------

static void handleTimeSave()
{
    if (!httpServer.hasArg("ntp1") ||
        !httpServer.hasArg("ntp2") ||
        !httpServer.hasArg("timezone"))
    {
        httpServer.send(
            400,
            "text/plain; charset=utf-8",
            "NTP Server und Zeitzone fehlen."
        );

        return;
    }

    String server1 = httpServer.arg("ntp1");
    String server2 = httpServer.arg("ntp2");
    String timezoneSelection = httpServer.arg("timezone");
    String customTimezone = httpServer.arg("customTimezone");

    server1.trim();
    server2.trim();
    customTimezone.trim();

    String timezone;

    if (timezoneSelection == "custom")
    {
        timezone = customTimezone;
    }
    else
    {
        timezone = timezoneSelection;
    }

    timezone.trim();

    if (server1.length() == 0 ||
        server2.length() == 0 ||
        timezone.length() == 0)
    {
        httpServer.send(
            400,
            "text/html; charset=utf-8",
            "<!DOCTYPE html>"
            "<html lang='de'>"
            "<head><meta charset='UTF-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
            "<title>DCM-77 Emulator</title></head>"
            "<body>"
            "<h1>Fehler</h1>"
            "<p>NTP Server und Zeitzone duerfen nicht leer sein.</p>"
            "<p><a href='/'>Zurueck</a></p>"
            "</body>"
            "</html>"
        );

        return;
    }

    saveTimeConfiguration(server1, server2, timezone);

    httpServer.send(
        200,
        "text/html; charset=utf-8",
        "<!DOCTYPE html>"
        "<html lang='de'>"
        "<head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>DCM-77 Emulator</title></head>"
        "<body>"
        "<h1>Zeitkonfiguration gespeichert</h1>"
        "<p>Die neuen NTP- und Zeitzoneneinstellungen wurden gespeichert.</p>"
        "<p>Der ESP32 startet jetzt neu.</p>"
        "</body>"
        "</html>"
    );

    delay(1000);

    ESP.restart();
}


// ------------------------------------------------------------
// NTP / Zeitzone auf Werkseinstellungen zurücksetzen
// ------------------------------------------------------------

static void handleTimeReset()
{
    resetTimeConfiguration();

    httpServer.send(
        200,
        "text/html; charset=utf-8",
        "<!DOCTYPE html>"
        "<html lang='de'>"
        "<head><meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
        "<title>DCM-77 Emulator</title></head>"
        "<body>"
        "<h1>Werkseinstellungen geladen</h1>"
        "<p>NTP Server und Zeitzone wurden auf die Werte aus "
        "config.h zurückgesetzt.</p>"
        "<p>Der ESP32 startet jetzt neu.</p>"
        "</body>"
        "</html>"
    );

    delay(1000);

    ESP.restart();
}


// ------------------------------------------------------------
// HTTP-Server Task
// ------------------------------------------------------------

static void httpServerTask(void *parameter)
{
    for (;;)
    {
        httpServer.handleClient();

        vTaskDelay(pdMS_TO_TICKS(2));
    }
}


// ------------------------------------------------------------
// HTTP-Server initialisieren
// ------------------------------------------------------------

void initHttpServer()
{
    httpServer.on("/", HTTP_GET, handleRoot);
    httpServer.on("/save", HTTP_POST, handleSave);
    httpServer.on("/clear", HTTP_POST, handleClear);

    httpServer.on("/timesave", HTTP_POST, handleTimeSave);
    httpServer.on("/timereset", HTTP_POST, handleTimeReset);

    httpServer.onNotFound([]()
    {
        httpServer.send(
            404,
            "text/plain; charset=utf-8",
            "Seite nicht gefunden."
        );
    });

    httpServer.begin();

    if (DEBUG_SERIAL) Serial.println("HTTP-Server gestartet.");

    if (isAccessPointMode())
    {
        if (DEBUG_SERIAL) Serial.print("Setup-Adresse: http://");
        if (DEBUG_SERIAL) Serial.println(WiFi.softAPIP());
    }
    else if (isWiFiConnected())
    {
        if (DEBUG_SERIAL) Serial.print("Webseite: http://");
        if (DEBUG_SERIAL) Serial.println(WiFi.localIP());
    }

    xTaskCreatePinnedToCore(
        httpServerTask,
        "HttpServer",
        8192,
        nullptr,
        1,
        &httpServerTaskHandle,
        0
    );
}
