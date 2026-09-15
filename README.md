# Auerswald DCM-77 Emulator

ESP32-basierter Ersatz für den historischen ACEM-DCF77-Empfänger
einer Auerswald DCM-77 Funkuhr.

## Ziel

Der historische DCF77-Empfänger der Uhr wird durch einen ESP32 ersetzt.

Der ESP32:

1. verbindet sich mit dem WLAN,
2. bezieht die aktuelle Zeit per NTP,
3. verwendet die deutsche Zeitzone,
4. erzeugt daraus ein DCF77-Telegramm,
5. gibt das demodulierte DCF77-Signal an GPIO12 aus.

Es wird **kein 77,5-kHz-Träger** erzeugt.

GPIO12 liefert ausschließlich das demodulierte Signal.

## Hardware

- ESP32-WROOM-32
- GPIO12 als Ausgang
- GPIO12 steuert über zwei Transistoren den Eingang der Uhr
- hier erfolgt eine Pegelanpassung auf 5V

## Arduino IDE

Das Projekt ist für die Arduino IDE mit einem ESP32-Board
ausgelegt.

Getestete Board-Umgebung:

- ESP32 Dev Module
- ESP32 Arduino Core 3.x

## Zeitquelle

Die Zeit wird per NTP bezogen.

Verwendete Server:

- pool.ntp.org
- time.nist.gov

## Zeitzone

Für Deutschland wird folgende POSIX-Zeitzonendefinition verwendet:

CET-1CEST,M3.5.0,M10.5.0/3

Damit erfolgt die automatische Umschaltung zwischen:

- MEZ
- MESZ

## DCF77-Ausgabe

Das Telegramm wird über GPIO12 ausgegeben.

### Bit 0

100 ms aktiv  
900 ms inaktiv

### Bit 1

200 ms aktiv  
800 ms inaktiv


## Telegramminhalt

Übertragen werden:

- Minute
- Stunde
- Tag
- Wochentag
- Monat
- Jahr
- Sommer-/Winterzeit
- Ankündigung der Zeitumstellung
- Paritätsbits

Nicht verwendete Bits werden auf 0 gesetzt.

## Projektstruktur

    Auerswald_DCM77_Emulator/
    │
    ├── Auerswald_DCM77_Emulator.ino
    ├── config.h
    │
    ├── dcf77.cpp
    ├── dcf77.h
    │
    ├── ntp.cpp
    ├── ntp.h
    │
    ├── timezone.cpp
    ├── timezone.h
    │
    └── README.md

### Auerswald_DCM77_Emulator.ino

Enthält:

- setup()
- loop()

### config.h

Enthält:

- GPIO-Konfiguration
- WLAN-Zugangsdaten
- NTP-Server
- Zeitzoneninformation

### dcf77.cpp / dcf77.h

Enthält:

- DCF77-Telegrammerzeugung
- Paritätsberechnung
- Bitübertragung
- GPIO-Ausgabe
- Minuten-Synchronisation

### ntp.cpp / ntp.h

Enthält:

- WLAN-Verbindung
- NTP-Konfiguration
- Warten auf gültige Zeit

### timezone.cpp / timezone.h

Enthält:

- Berechnung des DCF77-A1-Bits
- Erkennung der bevorstehenden Sommer-/Winterzeitumstellung

## Aktueller Entwicklungsstand
- Verbinden mit fest konfiguriertem WLAN
- Zeit über NTP bestimmen
- generieren des kompletten DCF77 Signals
- Senden des DCF77 Signals über GPIO12
- Start des Signals exakt bei Sekunde "00"
- DCF77 Telegramm enthält die folgenden Minute
- Automatische Umstellen Sommer- / Winterzeit

## Geplante Features
- Watchdog für NTP Synchronisation
- WLAN konfigurierbar
- Zeitzone konfigurierbar
- NTP Server konfigurierbar