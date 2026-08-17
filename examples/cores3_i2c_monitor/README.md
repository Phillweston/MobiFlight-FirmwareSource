# A320 OVHD CoreS3 ESPHome bridge

This firmware turns an M5Stack CoreS3 into an ESPHome device for the FENIX A320
overhead panel. Home Assistant discovers it directly through the ESPHome Native
API. The previous custom MQTT, Node-RED, captive-portal UI, and PlatformIO
firmware are no longer part of this project.

## Features

- 154 named digital input entities generated from the V1_0_2 `.mfproj` file.
- Six analog controls, each exposed as percentage and raw ADC entities.
- 151 shift-register outputs and seven ordinary-output state entities.
- BAT1 and BAT2 display text reconstructed from segment-display I2C frames.
- ESPHome Native API, OTA, web server, fallback AP, captive portal, and Improv
  Serial provisioning.
- CoreS3 touch UI with `STATUS`, `INPUT`, `ANALOG`, `OUTPUT`, and `SETUP` pages.
  Swipe vertically on the input and output pages to inspect every entity.

## Wiring

| Arduino I2C | CoreS3 Port A | Notes |
| --- | --- | --- |
| SDA | GPIO2 | Data |
| SCL | GPIO1 | Clock, 400 kHz |
| GND | GND | Common ground is required |

The Arduino is the I2C master. The CoreS3 is slave address `0x42`. Do not pull
the CoreS3 I2C side up to 5 V; use a bidirectional level shifter with 5 V Arduino
boards.

## Configure

Copy `esphome/secrets.yaml.example` to `esphome/secrets.yaml`, then generate a
valid ESPHome API encryption key and set the OTA and setup-hotspot passwords.
Wi-Fi credentials are not compiled into the firmware. `secrets.yaml` is ignored
by Git.

On first boot the device immediately starts the `A320 OVHD Setup` hotspot.
Connect using the configured setup password; the captive portal opens at
`http://192.168.4.1`, scans nearby networks, and lets the user select Wi-Fi
without typing its SSID. The selected credentials are saved on the CoreS3, so
later boots connect to that network automatically. Improv Serial can also
provision Wi-Fi over USB.

After connection, browse to `http://a320-ovhd-cores3.local`. In Home Assistant,
open **Settings > Devices & services** and accept the discovered ESPHome device.
No MQTT Broker, MQTT credentials, Node-RED flow, or Mi Home device fields are
required.

## Build and flash

Install `uv`, connect the CoreS3 by USB, and run:

```powershell
.\build_and_flash.ps1 -Port COM37
```

Compile without flashing:

```powershell
.\build_and_flash.ps1 -CompileOnly
```

The script runs ESPHome 2026.7 or newer through `uvx`. Subsequent updates can
also use OTA by passing the device hostname or IP to ESPHome.

## Regenerate entities

After changing `docs/FENIX A320 OVHD 自锁版 V1_0_2.mfproj`, regenerate and
validate the ESPHome package:

```powershell
python .\esphome\generate_config.py
uvx --python 3.13 esphome config .\esphome\a320-ovhd.yaml
```

The `.mfproj` uses symbolic names for the seven ordinary outputs and does not
record their physical Arduino GPIO numbers. Their ESPHome entities therefore
represent the I2C protocol channel mapping (`0` through `6`), not independently
verified Arduino pins. Confirm those channels with captured I2C frames on the
actual panel before relying on them.

Compilation verifies source and configuration compatibility only. It does not
prove CoreS3 display, touch, electrical-level, I2C timing, or complete panel
behavior on hardware.
