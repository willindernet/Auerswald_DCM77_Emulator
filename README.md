# Auerswald DCM-77 Emulator

ESP32-based replacement for the historic ACEM-DCF77 receiver of an Auerswald DCM-77 radio-controlled clock.

## Purpose

The clock's historic DCF77 receiver is replaced by an ESP32.

The ESP32:

1. connects to the configured Wi-Fi network,
2. obtains the current time via NTP from the configured NTP time servers,
3. uses the configured time zone,
4. generates a DCF77 telegram from it,
5. outputs the demodulated DCF77 signal on GPIO12.
6. monitors NTP synchronization and reconnects to Wi-Fi and resynchronizes NTP in the event of an error.

No **77.5 kHz carrier** is generated; GPIO12 provides only the demodulated signal.

## Hardware

- ESP32-WROOM-32
- GPIO12 as output
- GPIO12 controls the clock input via two transistors
- these transistors provide level shifting to 5V

## Arduino IDE

The project is designed for the Arduino IDE with an ESP32 board.

Tested board environment:

- ESP32 Dev Module
- ESP32 Arduino Core 3.x

## Time Source

The time is obtained via NTP.

Servers used:

- `pool.ntp.org` (Default, configurable)
- `time.nist.gov` (Default, configurable)

## Time Zone

The following POSIX time zone definition is used for Germany:

`CET-1CEST,M3.5.0,M10.5.0/3` (Default, configurable)

## Watchdog

The NTP watchdog independently checks the freshness of the last successful NTP synchronization.
- The watchdog checks the synchronization status every **1 minute**.
- A successful NTP synchronization remains valid for **6 hours**.
- If no successful synchronization has occurred within this period, DCF77 transmission is disabled immediately.
- If a DCF77 telegram is currently being transmitted, the transmission is aborted.
- The watchdog then attempts to restore time synchronization. If the Wi-Fi connection is interrupted, the saved Wi-Fi configuration is used to restore the connection.
- DCF77 transmission is not re-enabled until a successful NTP synchronization has occurred.
- An interrupted telegram is not resumed. Transmission starts again in a new minute cycle.

The watchdog therefore operates independently of the normal DCF77 transmission loop.

## DCF77 Output

The telegram is output via GPIO12.

### Bit 0

100 ms active  
900 ms inactive

### Bit 1

200 ms active  
800 ms inactive


### Telegram Content

The following are transmitted:

- Minute
- Hour
- Day
- Day of week
- Month
- Year
- Daylight saving time / standard time
- Announcement of the time change
- Parity bits

Unused bits are set to 0.

The 59 data bits are transmitted during seconds **0 to 58**. Second 59 contains no pulse and serves as the minute marker.

## Direct NTP Time Synchronization

The current version uses a direct NTP request via UDP port 123.

The received NTP time is directly applied to the ESP32 system time after successful validation. This makes operation independent of the automatic ESP32 SNTP synchronization.

DCF77 output is enabled only if, since the current startup, a successful NTP synchronization has been performed. Any existing system time alone is not sufficient.

## Time Zones

The web interface provides a selection of common time zones from Europe, North and South America, Africa, Asia, as well as Australia and New Zealand.

A custom POSIX TZ string can still be entered as well.

The values are stored persistently in NVS. Using "Load factory settings for NTP / time zone" restores the values from config.h.

Note: The DCF77 A1 calculation continues to follow the German DCF77 rule for announcing the European daylight saving time / standard time change.
The time zone selection changes the local time basis; the existing DCF77 telegram-logic has otherwise not been changed.

## Serial Output

Debug output on the serial console is disabled in the current version. This can be changed in config.h using `DEBUG_SERIAL`

## Wi-Fi Configuration

The Wi-Fi credentials are stored persistently in the non-volatile storage (NVS) of the ESP32.

At startup, the ESP32 first attempts to connect using the saved credentials.

If no credentials are stored or the connection cannot be established within the configured timeout, the `DCM77-Setup` is started automatically.

The access point uses the following address by default:

`http://192.168.4.1`

The SSID and password can be entered and saved via the web page. The ESP32 then restarts automatically.

The HTTP server runs in its own FreeRTOS task so that the DCF77 time transmission in the main program is not affected by processing HTTP requests.

## HTTP Server

The HTTP server is started automatically both in normal Wi-Fi operation and in access point mode.

In normal Wi-Fi operation, the web page is accessible at the IP address assigned by the router.

The web page provides:

- Display of Wi-Fi status
- Display of the IP address
- Display of signal strength in normal Wi-Fi operation
- Entering a new SSID
- Entering a new password
- Deletion of the saved Wi-Fi configuration
- Entering the NTP servers
- Entering the time zone
- Automatic restart after changing the configuration

## NTP

The time is retrieved directly via the NTP protocol using UDP port 123.

The current implementation does **not** use the ESP32's automatic SNTP synchronization. Instead, the NTP request is performed directly and the ESP32 system time is set based on the received NTP timestamp.

## Project Structure

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

Contains:

- `setup()`
- `loop()`

### config.h

Contains:

- Serial monitor configuration
- GPIO configuration
- NTP server factory configuration
- Time zone factory configuration
- Access Point name
- Wi-Fi connection timeout
- Watchdog

### dcf77.cpp / dcf77.h

Contains:

- DCF77 telegram generation
- Parity calculation
- Bit transmission
- GPIO output
- Minute synchronization
- Announcement of the time change

### httpserver.cpp / httpserver.h

Contains:

- Contains the HTTP server:
  - Wi-Fi configuration
  - NTP configuration

### ntp.cpp / ntp.h

Contains:

- Connecting to Wi-Fi
- Connecting to NTP server
- Synchronization

### watchdog.cpp / watchdog.h

Contains:

- Monitoring NTP synchronization
- Disabling the DCF77 signal when synchronization is missing
- Enabling the DCF77 signal after successful synchronization

## Installation / First Start

1. Install the ESP32 board support in the Arduino IDE.
2. Open the project and select the appropriate ESP32 board, e.g. `ESP32 Dev Module`.
3. Upload the firmware to the ESP32.
4. On first startup, or if no valid Wi-Fi configuration is stored, the ESP32 starts the open access point `DCM77-Setup`.
5. Connect to this access point and open `192.168.4.1` in a web browser.
6. Enter the Wi-Fi credentials and save the configuration.
7. The ESP32 restarts and connects to the configured Wi-Fi network.
8. After a successful NTP synchronization, DCF77 transmission is enabled.
9. Connect GPIO12 to the clock input via the intended transistor/level-shifting circuit. The ESP32 GPIO is not intended to drive the clock input directly.

The DCF77 output generated by this project is the demodulated pulse signal. The ESP32 does not generate a 77.5 kHz carrier

## Disclaimer

This is an independent, community-developed project and is not affiliated
with, endorsed by, sponsored by, or officially supported by Auerswald or Conrad.

The names Auerswald, Conrad, DCM-77, and ACEM are used solely
to identify the equipment and systems with which this project is intended to
be compatible or related.

This project is provided for informational, experimental, and personal use.
No guarantee is given that the emulator will work with every device,
hardware revision, or software/firmware version.

Use of this project is at your own risk. The project authors and contributors
are not responsible for any damage to hardware, loss of data, or other
consequences resulting from its use.

## License

The original source code of this project is licensed under the MIT License. See the [`LICENSE`](LICENSE) file for the full license text.

This project uses third-party components provided by the ESP32 Arduino Core and its dependencies. These components remain subject to their respective licenses and copyright notices.

See [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md) for an overview of the third-party components and their applicable licenses.

The MIT License of this project does not relicense or replace the licenses of third-party components.

