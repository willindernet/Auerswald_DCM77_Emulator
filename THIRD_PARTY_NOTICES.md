# Third-party notices

The **Auerswald DCM77 Emulator** is original project code and is licensed
under the MIT License. See the `LICENSE` file in this repository.

The project uses components provided by the ESP32 Arduino Core and its
dependencies. These third-party components remain subject to their
respective copyright and license terms.

This file documents the relevant third-party components used by the project.
It does not replace or modify the original license terms of those components.

## Arduino-ESP32 Core

This project is developed and tested with the Arduino-ESP32 core.

Upstream project:

https://github.com/espressif/arduino-esp32

The Arduino-ESP32 project is licensed under the GNU Lesser General Public
License, version 2.1 or later (LGPL-2.1-or-later). The repository also
contains individual components with their own license notices.

License information:

https://github.com/espressif/arduino-esp32/blob/master/LICENSE.md

The Arduino-ESP32 source tree is not included in this repository. It is
installed separately through the Arduino development environment.

## WebServer

The project uses the `WebServer` library supplied by the Arduino-ESP32 core:

```cpp
#include <WebServer.h>
```

The WebServer library is licensed under the GNU Lesser General Public
License, version 2.1 (LGPL-2.1).

Copyright notice in the upstream source:

Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.

Upstream source:

https://github.com/espressif/arduino-esp32/tree/master/libraries/WebServer

The license notice is included in the upstream source files, including
`WebServer.h` and `WebServer.cpp`.

The project does not copy the WebServer implementation into this repository.
It uses the library provided by the Arduino-ESP32 installation.

## Preferences

The project uses the `Preferences` library supplied by the Arduino-ESP32 core:

```cpp
#include <Preferences.h>
```

The Preferences library is licensed under the Apache License, Version 2.0.

Copyright notice in the upstream source:

Copyright 2015-2021 Espressif Systems (Shanghai) PTE LTD

Upstream source:

https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences

The license notice is included in the upstream source files.

The project does not copy the Preferences implementation into this
repository. It uses the library provided by the Arduino-ESP32 installation.

## WiFi and UDP networking

The project uses networking functionality supplied by the Arduino-ESP32 core,
including:

```cpp
#include <WiFi.h>
#include <WiFiUdp.h>
```

These components are part of the Arduino-ESP32 software stack and are
therefore subject to the copyright and license notices of their respective
upstream source files.

Upstream project:

https://github.com/espressif/arduino-esp32

No copies of the corresponding library source code are included in this
repository.

## Arduino framework

The project uses the Arduino framework through the ESP32 Arduino core,
including:

```cpp
#include <Arduino.h>
```

The applicable copyright and license terms are those of the Arduino-ESP32
core and the corresponding Arduino components used by the build.

## FreeRTOS

The project uses FreeRTOS task functionality through the ESP32 software
platform.

FreeRTOS is distributed under the MIT open-source license.

Upstream project:

https://www.freertos.org/

License information:

https://freertos.org/Documentation/02-Kernel/01-About-the-FreeRTOS-kernel/04-Licensing

The FreeRTOS source code is not included in this repository.

## Relationship to this project's MIT License

The MIT License in the `LICENSE` file applies to the original source code of
the Auerswald DCM77 Emulator.

It does not relicense, replace, or remove the licenses of third-party
components used by the project.

Third-party components remain subject to their respective licenses.

When distributing firmware or other products containing third-party
components, the applicable license obligations for those components must be
observed.

## No bundled third-party source code

This repository does not intentionally include copies of the Arduino-ESP32
`WebServer`, `Preferences`, WiFi, or FreeRTOS source trees.

These components are dependencies supplied by the ESP32/Arduino software
environment used to compile the project.

The exact versions and license terms applicable to a particular build depend
on the Arduino-ESP32 core and associated dependencies installed in the
development environment.

## Adding future dependencies

If additional third-party libraries or source code are added to the project,
their copyright and license terms should be reviewed and documented here
where appropriate.

This document is a technical project-level license notice and is not legal
advice.
