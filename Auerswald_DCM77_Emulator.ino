#include "config.h"
#include "ntp.h"
#include "dcf77.h"

void setup()
{
    initDCF();

    Serial.begin(115200);

    connectWiFi();

    initNTP();

    waitForTime();
}

void loop()
{
    struct tm now;

    // Überprüfen der Variable now
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

    // Nur am Anfang einer Minute neues Telegramm erzeugen
    createDCF77Telegram(now);

    // Warten auf Sekunde "0"
    waitForNextMinute();

    // Hier werden die Bits der Zeitinformation geschickt
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