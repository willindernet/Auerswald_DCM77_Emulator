#include "config.h"
#include "ntp.h"
#include "dcf77.h"
#include "httpserver.h"
#include "watchdog.h"

#include <Arduino.h>
#include <time.h>


void setup()
{
    // DCF77-Ausgang initialisieren
    initDCF();

    if (DEBUG_SERIAL) Serial.begin(115200);

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

    initNtpWatchdog();
}


void loop()
{
    struct tm now;

    // DCF77 darf ausschließlich bei gültiger NTP-Synchronisation senden.
    if (!isDcf77TransmissionAllowed())
    {
        dcfInactive();
        delay(100);
        return;
    }

    // Ein Abbruchereignis gehört immer zum vorherigen Telegramm.
    // Jetzt beginnt ein neuer Telegrammzyklus.
    clearDcf77TransmissionAbortRequest();

    if (!getLocalTime(&now))
    {
        dcfInactive();
        delay(100);
        return;
    }

    // Bestehende DCF77-Telegrammlogik unverändert.
    createDCF77Telegram(now);

    // Auf den nächsten Minutenbeginn warten.
    waitForNextMinute();

    // Während des Wartens kann der Watchdog die NTP-Gültigkeit verlieren.
    // In diesem Fall KEIN Telegramm beginnen, sondern sofort zum nächsten
    // loop()-Durchlauf zurückkehren.
    if (!isDcf77TransmissionAllowed() ||
        isDcf77TransmissionAbortRequested())
    {
        dcfInactive();
        return;
    }

    // 59 Sekunden DCF77-Daten senden.
    for (int second = 0; second < 59; second++)
    {
        if (dcfBits[second])
            sendBit1();
        else
            sendBit0();

        // Der unabhängige Watchdog darf ein laufendes Telegramm jederzeit
        // abbrechen. Nach dem return startet loop() einen neuen Zyklus.
        if (!isDcf77TransmissionAllowed() ||
            isDcf77TransmissionAbortRequested())
        {
            dcfInactive();
            return;
        }
    }
}
