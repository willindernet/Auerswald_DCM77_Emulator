#include "httpserver.h"
#include "ntp.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

static WebServer httpServer(80);
static TaskHandle_t httpServerTaskHandle = nullptr;


// ------------------------------------------------------------
// HTML-Seite
// ------------------------------------------------------------

static String makePage()
{
    String html;

    html += "<!DOCTYPE html>";
    html += "<html lang='de'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>DCM-77 Emulator</title>";

    html += "<style>";
    html += "body{font-family:Arial,sans-serif;max-width:600px;margin:40px auto;padding:20px;}";
    html += "h1{font-size:24px;}";
    html += "label{display:block;margin-top:15px;}";
    html += "input{width:100%;padding:10px;margin-top:5px;box-sizing:border-box;}";
    html += "button{margin-top:20px;padding:12px 20px;font-size:16px;}";
    html += ".status{padding:15px;background:#eee;margin:20px 0;}";
    html += "</style>";

    html += "</head>";
    html += "<body>";

    html += "<h1>Auerswald DCM-77 Emulator</h1>";

    html += "<div class='status'>";

    if (isAccessPointMode())
    {
        html += "<b>WLAN-Modus:</b> Access Point<br>";
        html += "<b>SSID:</b> ";
        html += AP_SSID;
        html += "<br>";
        html += "<b>IP-Adresse:</b> ";
        html += WiFi.softAPIP().toString();
    }
    else if (isWiFiConnected())
    {
        html += "<b>WLAN-Modus:</b> verbunden<br>";
        html += "<b>SSID:</b> ";
        html += WiFi.SSID();
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

    html += "<h2>WLAN konfigurieren</h2>";

    html += "<form action='/save' method='POST'>";

    html += "<label>SSID</label>";
    html += "<input type='text' name='ssid' required>";

    html += "<label>Passwort</label>";
    html += "<input type='password' name='password'>";

    html += "<button type='submit'>Speichern und Neustart</button>";

    html += "</form>";

    html += "<form action='/clear' method='POST'>";
    html += "<button type='submit'>WLAN-Konfiguration löschen</button>";
    html += "</form>";

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
    if (!httpServer.hasArg("ssid"))
    {
        httpServer.send(
            400,
            "text/plain; charset=utf-8",
            "SSID fehlt."
        );

        return;
    }

    String ssid = httpServer.arg("ssid");
    String password = httpServer.arg("password");

    ssid.trim();

    if (ssid.length() == 0)
    {
        httpServer.send(
            400,
            "text/plain; charset=utf-8",
            "SSID darf nicht leer sein."
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
// HTTP-Server Task
//
// Der Server läuft in einem eigenen FreeRTOS-Task.
// Dadurch wird die DCF77-Ausgabe im loop() nicht durch
// handleClient() unterbrochen.
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

    httpServer.onNotFound([]()
    {
        httpServer.send(
            404,
            "text/plain; charset=utf-8",
            "Seite nicht gefunden."
        );
    });

    httpServer.begin();

    Serial.println("HTTP-Server gestartet.");

    if (isAccessPointMode())
    {
        Serial.print("Setup-Adresse: http://");
        Serial.println(WiFi.softAPIP());
    }
    else if (isWiFiConnected())
    {
        Serial.print("Webseite: http://");
        Serial.println(WiFi.localIP());
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
