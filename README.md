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
    ├── watchdog.cpp
    ├── watchdog.h
    │
    ├── timezone.cpp
    ├── timezone.h
    │
    ├── httpserver.cpp
    ├── httpserver.h
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

## Direkte NTP-Zeitsynchronisation

Die aktuelle Version verwendet eine direkte NTP-Anfrage über UDP Port 123.
Die empfangene NTP-Zeit wird nach erfolgreicher Prüfung direkt in die
ESP32-Systemzeit übernommen. Dadurch ist der Betrieb unabhängig von der
automatischen ESP32-SNTP-Synchronisation.

Die DCF77-Ausgabe wird nur freigegeben, wenn seit dem aktuellen Start eine
erfolgreiche NTP-Synchronisation durchgeführt wurde. Eine eventuell noch
vorhandene Systemzeit allein reicht nicht aus.

## Zeitzonen

Die Weboberfläche enthält eine Auswahl gängiger Zeitzonen aus Europa,
Nord- und Südamerika, Afrika, Asien sowie Australien und Neuseeland.
Zusätzlich kann weiterhin eine eigene POSIX-TZ-Zeichenkette eingetragen werden.

Hinweis: Die DCF77-A1-Berechnung folgt weiterhin der deutschen DCF77-Regel
für die Ankündigung der europäischen Sommer-/Winterzeitumstellung. Die
Zeitzonenauswahl ändert die lokale Zeitbasis; die bestehende DCF77-Telegramm-
logik wurde ansonsten nicht verändert.

## Serielle Ausgabe

Die Debug-Ausgaben auf der seriellen Konsole sind in der aktuellen Version
deaktiviert. Die Funktionalität des Emulators ist davon unabhängig.

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

## WLAN-Konfiguration

Die WLAN-Zugangsdaten werden dauerhaft im nichtflüchtigen Speicher
(NVS) des ESP32 mit `Preferences` gespeichert.

Beim Start versucht der ESP32 zunächst, sich mit den gespeicherten
Zugangsdaten zu verbinden.

Wenn keine Zugangsdaten gespeichert sind oder die Verbindung innerhalb
des konfigurierten Zeitlimits nicht gelingt, wird automatisch der
Access Point

    DCM77-Setup

gestartet.

Der Access Point verwendet standardmäßig die Adresse:

    http://192.168.4.1

Über die Webseite können SSID und Passwort eingegeben und gespeichert
werden. Anschließend startet der ESP32 automatisch neu.

Der HTTP-Server läuft in einem eigenen FreeRTOS-Task, damit die
DCF77-Zeitübertragung im Hauptprogramm nicht durch die Bearbeitung von
HTTP-Anfragen verändert wird.

## HTTP-Server

Der HTTP-Server wird sowohl im normalen WLAN-Betrieb als auch im
Access-Point-Modus automatisch gestartet.

Im normalen WLAN-Betrieb ist die Webseite unter der vom Router
vergebenen IP-Adresse erreichbar.

Die Webseite bietet:

- Anzeige des WLAN-Status
- Anzeige der IP-Adresse
- Anzeige der Signalstärke im normalen WLAN-Betrieb
- Eingabe einer neuen SSID
- Eingabe eines neuen Passworts
- Löschen der gespeicherten WLAN-Konfiguration
- automatischen Neustart nach Änderung der Konfiguration

## Dateien

### httpserver.cpp / httpserver.h

Enthält den HTTP-Server und die WLAN-Konfigurationsseite.

Die Dateien heißen bewusst `httpserver.*`. Dadurch gibt es keine
Namenskollision mit Include-Guards einer möglichen `WebServer.h`-
Bibliothek.


## NTP- und Zeitzonenkonfiguration

Die NTP-Server und die POSIX-Zeitzone können über die Weboberfläche konfiguriert werden. Die Werte werden dauerhaft in NVS gespeichert. Über „Werkseinstellungen für NTP / Zeitzone laden“ werden die Werte aus `config.h` wieder verwendet. Die Speicherung erfolgt mit der ESP32-Preferences/NVS-Funktion. citeturn0search0


### NTP / Zeitzone

NTP-Server und Zeitzone werden dauerhaft in Preferences gespeichert. Beim Start wird `configTzTime()` verwendet, damit die gespeicherten NTP-Server und die POSIX-Zeitzone gemeinsam an den ESP32-SNTP-Dienst übergeben werden.


### NTP-Initialisierung
Die konfigurierten NTP-Server werden mit `configTime()` gesetzt; die konfigurierbare POSIX-Zeitzone wird anschließend über `TZ`/`tzset()` gesetzt. Damit entspricht die Initialisierung wieder der zuvor nachweislich funktionierenden Variante.

### NTP-Diagnose (v6)

Die serielle Konsole gibt zusätzlich WLAN-IP, Gateway, DNS-Server,
DNS-Auflösung der konfigurierten NTP-Server, die tatsächlich an SNTP
übergebenen Server sowie den SNTP-Synchronisationsstatus aus.


## v8 – direkte NTP-Zeitsynchronisation

v8 verwendet nach erfolgreicher WLAN- und DNS-Verbindung eine direkte NTP-Anfrage über UDP/123. Die gültige NTP-Serverantwort wird ausgewertet und die ESP32-Systemzeit mit `settimeofday()` gesetzt. Die bisherige ESP32-SNTP-Automatik über `configTime()` wird in v8 nicht benötigt. Server 2 dient als Fallback, falls Server 1 keine gültige Antwort liefert. Anschließend wird die konfigurierte POSIX-Zeitzone über `TZ`/`tzset()` aktiviert. Die DCF77-Telegrammlogik und deren Timing bleiben unverändert.


## v9 – Sicherheitslogik für die DCF77-Ausgabe

Die DCF77-Ausgabe wird ab v9 ausschließlich freigegeben, wenn seit dem aktuellen
Start eine direkte NTP-Synchronisation erfolgreich war. `getLocalTime()` allein
wird nicht mehr als Freigabekriterium verwendet, da die ESP32-Systemuhr nach einem
Neustart noch eine alte/erhaltene Zeit enthalten kann.

Wenn keine WLAN-Zugangsdaten vorhanden sind, die WLAN-Verbindung fehlschlägt oder
die NTP-Synchronisation fehlschlägt, bleibt der DCF77-Ausgang inaktiv. Der
Access-Point und die Weboberfläche bleiben zur Neueinrichtung verfügbar.


### Zyklische NTP-Sicherheitsprüfung

In `config.h` kann mit

`const unsigned long NTP_SYNC_TIMEOUT_SECONDS = 6UL * 60UL * 60UL;`

die maximal zulässige Zeit seit der letzten erfolgreichen NTP-Synchronisation
eingestellt werden. Nach Ablauf dieses Zeitraums wird die DCF77-Ausgabe
gesperrt. Der ESP32 versucht anschließend automatisch, die WLAN-Verbindung
wiederherzustellen und eine direkte UDP-NTP-Synchronisation mit dem ersten
oder zweiten konfigurierten NTP-Server durchzuführen. Erst nach Erfolg wird
die DCF77-Ausgabe wieder freigegeben.

## NTP-Watchdog

Die NTP-Überwachung läuft in einem eigenen FreeRTOS-Task und damit unabhängig
vom Arduino-Mainloop. `NTP_SYNC_TIMEOUT_SECONDS` ist standardmäßig auf 6 Stunden,
`NTP_WATCHDOG_INTERVAL_MS` auf 60 Sekunden gesetzt.

Wird die Synchronisation ungültig, wird GPIO12 sofort auf LOW gesetzt und ein
laufendes DCF77-Telegramm abgebrochen. Erst eine erfolgreiche erneute
NTP-Synchronisation gibt die DCF77-Ausgabe wieder frei.

Die bestehende DCF77-Telegrammlogik einschließlich `t += 120`, Bit-Timing und
Ausgabe ohne 77,5-kHz-Träger bleibt unverändert.


## v13 Watchdog-Test

Testwerte: `NTP_SYNC_TIMEOUT_SECONDS = 60UL` und
`NTP_WATCHDOG_INTERVAL_MS = 5000UL`.

Bei Ablauf wird ein laufendes Telegramm sofort abgebrochen. Eine erfolgreiche
NTP-Recovery gibt DCF77 wieder frei, löscht aber nicht das Abbruchereignis.
Dieses wird erst unmittelbar vor einem neuen Telegramm gelöscht.

Für Diagnose kann `DEBUG_SERIAL` testweise auf `1` gesetzt werden. Die
Watchdog-Meldungen enthalten die Uptime seit dem Einschalten sowie Timeout-
und Recovery-Zeitpunkte.

## v14 Minutenzyklus

Nach einem Watchdog-Abbruch wird der aktuelle DCF77-Zyklus sofort beendet.
Der nächste `loop()`-Durchlauf beginnt einen neuen Zyklus. Dadurch kann ein
abgebrochenes Telegramm nicht in einen falschen Minutenzyklus hineinwirken.
Die bestehende DCF77-Telegrammlogik bleibt unverändert.
