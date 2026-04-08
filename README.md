# cackalackybadgyfirmware2025
Firmware for the CackalackyBadgy from CackalackyCon 2025

![alt text](https://github.com/lockfale/cackalackybadgyfirmware2025/blob/main/media/badges.jpg)

Now that [CackalackyCon 2025](https://cackalackycon.org/index.html) is over, we'll post the developers version of the firmware that was used for debugging and testing as well as more information about the badge and capabilities.

# The Largely Unread Manual
At opening ceremony, everyone was directed to read the [manual](https://github.com/lockfale/cackalackybadgyfirmware2025/blob/main/media/cyberpartner_manual_v1.pdf). Sadly, very few did.

## Those responsible
### Hardware Design
* [@melvin2001](https://github.com/melvin2001)
### Firmware
* [@pandatrax](https://github.com/pandatrax)
* [@working-as-designed](https://github.com/working-as-designed)
* [@nutcrunch](https://github.com/persinac)
* [@melvin2001](https://github.com/melvin2001)
* [@clarkehackworth](https://github.com/clarkehackworth)
### Sprites
* [@Corvidra](https://github.com/GlitchesandGiggles)
* [@working-as-designed](https://github.com/working-as-designed)
### Everything Server/Infra related
* [@nutcrunch](https://github.com/persinac)

## Pre-built Firmware

Pre-built binaries are available in the [`firmware/`](firmware/) directory:

- `firmware/firmware.bin` — badge firmware
- `firmware/littlefs.bin` — LittleFS filesystem image

Flash both with:

```shell
esptool -b 921600 --chip esp8266 write_flash 0x0 firmware/firmware.bin 1048576 firmware/littlefs.bin
```

---

## Building & Development

### Prerequisites

All you need is [Docker](https://docs.docker.com/get-docker/) and a USB connection to the badge. Docker handles PlatformIO, esptool, and clang-tidy — no local installs required.

> **Note:** The script auto-detects `/dev/ttyACM0` or `/dev/ttyUSB0`. On Linux you may need to add yourself to the `dialout` group (`sudo usermod -aG dialout $USER`) or install [udev rules](https://docs.platformio.org/en/latest/core/installation/udev-rules.html) for USB flashing to work.

### 1. Build and start the container

Run this once from the repo root (also use it to rebuild after Dockerfile changes):

```shell
scripts/rebuild_docker.sh
```

Then open a shell inside the container:

```shell
docker exec -it esp32-dev bash
```

### 2. Build and flash (inside the container)

> **Before flashing:** hold the **Boot button** while plugging the badge into USB to put it in flash mode.

```shell
repo            # cd into the repo at /code/cacka
pio_build_all   # build firmware image + LittleFS
pio_flash       # flash firmware + LittleFS to the badge
```

Other useful aliases available inside the container:

| Alias | Command |
|---|---|
| `repo` | `cd /code/cacka` |
| `pio_build` | `pio run -v` |
| `pio_build_littlefs` | `pio run --target buildfs --jobs 30` |
| `pio_build_all` | build firmware + LittleFS |
| `clean_build` | clean, then build firmware + LittleFS |
| `pio_flash` | flash firmware + LittleFS via esptool |
| `pio_monitor` | `pio device monitor` (9600 baud) |

### Linting (inside the container)

```shell
repo
mkdir -p build && cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# lint a specific file
clang-tidy /code/cacka/src/specific_file.cpp -p /code/cacka/build --config-file=/code/cacka/.clang-tidy

# auto-fix
clang-tidy /code/cacka/src/specific_file.cpp -p /code/cacka/build --config-file=/code/cacka/.clang-tidy -fix -fix-errors
```

> **Note:** Ignore `'c_types.h' file not found` errors — these come from the ESP8266 Arduino core and are not real issues.

---

## Generating and Browsing Documentation

1. **Generate documentation** — run from the repo root:
   ```shell
   ./scripts/make_docs.sh
   ```

2. **Browse documentation** — starts a local web server:
   ```shell
   ./scripts/run_document_webserver.sh
   ```
   Then open `http://localhost:8000` in your browser.

---

## Serial Commands

Connect at 9600 baud (`pio_monitor` inside the container, or `pio device monitor` natively).

### Default commands

```
Help          — List available commands
Status        — Show progress and current state
Helloworld    — Example of how to use commands
su <word>     — Become a badge admin
PartnerStats  — Print your CyberPartner stats
```

### Admin commands

```
Delprog         — Delete all badge progress
ResetNetwork    — Reset WiFi and MQTT settings
SetWiFiSSID     — Set WiFi SSID
SetWiFiPass     — Set WiFi password
SetMQTTServer   — Set MQTT server address
SetMQTTPort     — Set MQTT port
SetMQTTUser     — Set MQTT username
SetMQTTPass     — Set MQTT password
SaveNetwork     — Save network config (restart required)
Restart         — Restart the badge
```

### NeoPixel commands

**Solid colors:** `Pink` `Red` `Green` `Blue` `Yellow` `White`

**Patterns:** `Chase` `Confetti` `Popo` `Rainbow` `Strobe` `Surge` `SC`

---

## Community & Support

Have questions about the badge? Join the [CackalackyCon Discord](https://discord.gg/4YJuZNE2a5) and ask in **#hhv-aka-thebadge**.

[CackalackyCon](https://cackalackycon.org/) is a hacker con based in the Carolinas — come hang out, learn things, and pick up next year's badge.

---

## Links
* [CackalackyCon](https://cackalackycon.org/index.html)
* [persinac's badge backend writeup](https://medium.com/@persinac/c-ck-l-cky-con-2025-d-day-technical-retro-4c445f3e2a3d)
* [working-as-designed's badge firmware writeup](https://working-as-designed.github.io/2025/05/28/cackalackybadgy-2025.html)
* [Cackalacky Badgy firmware 2024](https://github.com/lockfale/cackalackybadgyfirmware2024)
* [MQTT Example](https://github.com/lockfale/mqtt-example)
* [psmitty7373's Badge Client](https://github.com/psmitty7373/badge_client)
* [Mira's Badge Script](https://gist.github.com/ArgentumCation/bf65f4cf39e4c94eed7aa3b27dd20bce)
