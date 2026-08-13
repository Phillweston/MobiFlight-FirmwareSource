# MobiFlight-FirmwareSource

## ESP32 I2C event bridge

The firmware mirrors configured button changes and PC-issued digital output
changes to an ESP32 I2C slave at address `0x42`. The existing serial messages
and Arduino GPIO behavior are unchanged. Connect SDA to SDA, SCL to SCL, and
the grounds together. Use level conversion when the Arduino board uses 5 V.

Each event uses the version 2 length-prefixed frame documented in
[`docs/ESP32_I2C_BRIDGE_PROTOCOL.md`](docs/ESP32_I2C_BRIDGE_PROTOCOL.md). It
supports buttons, analog inputs, input multiplexers, input/output shift
registers, segment displays, LCD text, and ordinary digital outputs.

The address can be changed at compile time with
`-DMF_I2C_BRIDGE_ADDRESS=0xNN`.

PlatformIO version of the MobiFlight firmware source.

**Warning**: Before attempting to build the repo you must enable long path
support in git otherwise you will get a build failure on the Raspberry Pi Pico
build. To enable long paths type the following in a terminal window:

```powershell
git config --global core.longpaths true
```

If you still receive an error then [enable long paths support in Windows](https://www.thewindowsclub.com/how-to-enable-or-disable-win32-long-paths-in-windows-11-10).

## Building

1. Install [Visual Studio Code](https://code.visualstudio.com/Download)
2. Install the [PlatformIO extension](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide). Visual Studio Code will automatically
   suggest this extension if you don't already have it installed.
3. From the Command Palette select `PlatformIO: Build` to build or `PlatformIO: Upload` to build and upload to your connected board

If you want to speed up local development and only build for one of the supported platforms then click on the `Default (MobiFlight-FirmwareSource)` label
in the Visual Studio Code status bar and then select the specific target platform you want to build for.

## Publishing a release

To publish a new release:

1. Go to the [GitHub Releases page](https://github.com/MobiFlight/MobiFlight-FirmwareSource/releases) and
click the `Draft a new release` button.
2. Click `Choose a tag` and create a new version tag, e.g. `1.14.0`
3. Enter a title and release notes.
4. Optionally check `This is a pre-release` if the release should be for testing purposes and not be marked
as the latest published release.
5. Click `Publish release`.

The release build process will automatically run and after a few minutes attach firmware binaries
for the release to the release page.

If a release was marked as pre-release you can go back later and edit it to remove the pre-release designation
which will automatically promote it to the latest published release.
