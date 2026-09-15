#include "config.h"
#include "ntp.h"
#include "dcf77.h"
#include "httpserver.h"

#include <Arduino.h>
#include <time.h>


void setup()
{
    // DCF77-Ausgang initialisieren
    initDCF();

    Serial.begin(115200);

    // WLAN verbinden oder Access Point starten
    bool connected = connectWiFi();

    // Webserver immer starten
    initHttpServer();

    // NTP nur bei bestehender WLAN-Verbindung
    if (connected)
    {
        initNTP();
        waitForTime();
    }
}


void loop()
{
    struct tm now;

    // Wenn noch keine NTP-Zeit vorhanden ist,
    // nichts senden.
    if (!getLocalTime(&now))
    {
        delay(1000);
        return;
    }

    // Log aktuelle Zeit
    Serial.println();
    Serial.printf(
        "%02d:%02d:%02d\n",
        now.tm_hour,
        now.tm_min,
        now.tm_sec
    );

    // Telegramm erzeugen
    //
    // WICHTIG:
    // Die vorhandene Logik inklusive +120
    // bleibt in createDCF77Telegram() erhalten.
    createDCF77Telegram(now);

    // Warten auf Sekunde 00
    waitForNextMinute();

    // 59 Bits senden
    for (int second = 0; second < 59; second++)
    {
        if (dcfBits[second])
        {
            sendBit1();
        }
        else
        {
            sendBit0();
        }
    }
}