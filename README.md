# Auerswald DCM-77 Emulator

ESP32-basierter Ersatz für den historischen ACEM-DCF77-Empfänger einer Auerswald DCM-77 Funkuhr.

## Ziel

Der historische DCF77-Empfänger der Uhr wird durch einen ESP32 ersetzt.

Der ESP32:

1. verbindet sich mit dem konfigurierten WLAN,
2. bezieht die aktuelle Zeit per NTP über die konfigurierten NTP Zeitserver,
3. verwendet die konfigurierte Zeitzone,
4. erzeugt daraus ein DCF77-Telegramm,
5. gibt das demodulierte DCF77-Signal an GPIO12 aus.
6. überwacht die NTP Synchronisation und verbindet im Fehlerfall das WLAN neu und synchronisiert NTP.

Es wird **kein 77,5-kHz-Träger** erzeugt, GPIO12 liefert ausschließlich das demodulierte Signal.

## Hardware

- ESP32-WROOM-32
- GPIO12 als Ausgang
- GPIO12 steuert über zwei Transistoren den Eingang der Uhr
- mit diesen Transistoren erfolgt eine Pegelanpassung auf 5V

## Arduino IDE

Das Projekt ist für die Arduino IDE mit einem ESP32-Board ausgelegt.

Getestete Board-Umgebung:

- ESP32 Dev Module
- ESP32 Arduino Core 3.x

## Zeitquelle

Die Zeit wird per NTP bezogen.

Verwendete Server:

- `pool.ntp.org` (Default, konfigurierbar)
- `time.nist.gov` (Default, konfigurierbar)

## Zeitzone

Für Deutschland wird folgende POSIX-Zeitzonendefinition verwendet:

`CET-1CEST,M3.5.0,M10.5.0/3` (Default, konfigurierbar)

## Watchdog

Der NTP-Watchdog überprüft unabhängig die Aktualität der letzten erfolgreichen NTP-Synchronisation.
- Der Watchdog überprüft den Synchronisationsstatus alle **1 Minuten**.
- Eine erfolgreiche NTP-Synchronisation bleibt **6 Stunden** lang gültig.
- Wenn innerhalb dieses Zeitraums keine erfolgreiche Synchronisation stattgefunden hat, wird die DCF77-Übertragung sofort deaktiviert.
- Wird gerade ein DCF77-Telegramm übertragen, wird die Übertragung abgebrochen.
- Der Watchdog versucht anschließend, die Zeitsynchronisation wiederherzustellen. Ist die WLAN-Verbindung unterbrochen, wird die gespeicherte WLAN-Konfiguration verwendet, um die Verbindung wiederherzustellen.
- Die DCF77-Übertragung wird erst nach einer erfolgreichen NTP-Synchronisation wieder aktiviert.
- Ein unterbrochenes Telegramm wird nicht fortgesetzt. Die Übertragung beginnt in einem neuen Minutenzyklus erneut.

Der Watchdog arbeitet somit unabhängig von der normalen DCF77-Übertragungsschleife.
## DCF77-Ausgabe

Das Telegramm wird über GPIO12 ausgegeben.

### Bit 0

100 ms aktiv  
900 ms inaktiv

### Bit 1

200 ms aktiv  
800 ms inaktiv


### Telegramminhalt

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
Die 59 Datenbits werden in den Sekunden **0 bis 58** übertragen. Die Sekunde 59 enthält keinen Impuls und dient als Minutenmarkierung.

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
    ├── httpserver.cpp
    ├── httpserver.h
    │
    └── README.md

### Auerswald_DCM77_Emulator.ino

Enthält:

- `setup()`
- `loop()`

### config.h

Enthält:

- Debugging über den seriellen Monitor
- GPIO-Konfiguration
- NTP-Server
- Zeitzoneninformation
- Access Point Mode
- Timeout für WLAN-Verbindung
- Watchdog

### dcf77.cpp / dcf77.h

Enthält:

- DCF77-Telegrammerzeugung
- Paritätsberechnung
- Bitübertragung
- GPIO-Ausgabe
- Minuten-Synchronisation
- Ankündigung der Zeitumstellung

### httpserver.cpp / httpserver.h

Enthält:

- Enthält den HTTP-Server
- WLAN-Konfiguration
- NTP-Konfiguration

### ntp.cpp / ntp.h

Enthält:

- Verbinden mit WLAN
- Verbinden mit NTP Server
- Synchronisation

### watchdog.cpp / watchdog.h

Enthält:

- Überwachung der NTP Synchronisation
- Deaktivieren des DCF77 Signals bei fehlender Synchronisation
- Aktivieren des DCF77 Signals bei erfolgreicher Synchronisation

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

Die Werte werden dauerhaft in NVS gespeichert. Über "Werkseinstellungen für NTP / Zeitzone laden"
werden die Werte aus config.h wieder verwendet.

Hinweis: Die DCF77-A1-Berechnung folgt weiterhin der deutschen DCF77-Regel
für die Ankündigung der europäischen Sommer-/Winterzeitumstellung. Die
Zeitzonenauswahl ändert die lokale Zeitbasis; die bestehende DCF77-Telegramm-
logik wurde ansonsten nicht verändert.

## Serielle Ausgabe

Die Debug-Ausgaben auf der seriellen Konsole sind in der aktuellen Version
deaktiviert. Dies kann in der config.h mit `DEBUG_SERIAL` geändert werden

## WLAN-Konfiguration

Die WLAN-Zugangsdaten werden dauerhaft im nichtflüchtigen Speicher
(NVS) des ESP32 gespeichert.

Beim Start versucht der ESP32 zunächst, sich mit den gespeicherten
Zugangsdaten zu verbinden.

Wenn keine Zugangsdaten gespeichert sind oder die Verbindung innerhalb
des konfigurierten Zeitlimits nicht gelingt, wird automatisch der

`DCM77-Setup` (Default, konfigurierbar)

gestartet.

Der Access Point verwendet standardmäßig die Adresse:

`http://192.168.4.1`

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
- Eingabe der NTP-Server
- Eingabe der Zeitzone
- automatischen Neustart nach Änderung der Konfiguration

## NTP

Die Zeit wird direkt über das NTP-Protokoll unter Verwendung des UDP-Ports 123 abgerufen.

Die aktuelle Implementierung nutzt **nicht** die automatische SNTP-Synchronisation des ESP32. Stattdessen wird die NTP-Anfrage direkt durchgeführt und die Systemzeit des ESP32 anhand des empfangenen NTP-Zeitstempels eingestellt.

## Installation / Erster Start

1. Installieren Sie den ESP32-Board-Support in der Arduino-IDE.
2. Öffnen Sie das Projekt und wählen Sie das entsprechende ESP32-Board aus, z. B. `ESP32 Dev Module`.
3. Laden Sie die Firmware auf den ESP32 hoch.
4. Beim ersten Start oder wenn keine gültige WLAN-Konfiguration gespeichert ist, startet das ESP32 den offenen Zugangspunkt `DCM77-Setup`.
5. Stellen Sie eine Verbindung zu diesem Zugangspunkt her und öffnen Sie `192.168.4.1` in einem Webbrowser.
6. Geben Sie die WLAN-Zugangsdaten ein und speichern Sie die Konfiguration.
7. Das ESP32 startet neu und verbindet sich mit dem konfigurierten WLAN-Netzwerk.
8. Nach einer erfolgreichen NTP-Synchronisation wird die DCF77-Übertragung aktiviert.
9. Verbinden Sie GPIO12 über die vorgesehene Transistor-/Pegelumsetzerschaltung mit dem Takteingang. Der ESP32-GPIO ist nicht dafür vorgesehen, den Takteingang direkt anzusteuern.

Der von diesem Projekt erzeugte DCF77-Ausgang ist das demodulierte Impulssignal. Der ESP32 erzeugt keinen 77,5-kHz-Träger.
