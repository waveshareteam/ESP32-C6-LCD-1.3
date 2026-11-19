# LVGL weather clock

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Displays the time and weather on the ST7789 LCD. The sketch uses Wi-Fi to synchronize time and retrieve weather data.

The bundled [Arduino Time library](../../libraries/Time/) is licensed under the [GNU LGPL 2.1 or later](../../libraries/Time/LICENSE).

## Hardware Required

- ESP32-C6-LCD-1.3 development board and USB data cable
- 2.4 GHz Wi-Fi network with internet access

## How to Run

1. Install the Arduino-ESP32 version listed in the [repository README](../../../../README.md).
2. Set the sketchbook location to `examples/arduino/`, or copy the [bundled libraries](../../libraries/) into your sketchbook's `libraries` directory.
3. Select **ESP32C6 Dev Module** and match the board options in the [product configuration](../../../../.github/product.json).
4. Enter your Wi-Fi network name and password in [`WeatherClock.cpp`](WeatherClock.cpp) before uploading. Open [`LVGL_WeatherClock.ino`](LVGL_WeatherClock.ino) and upload.

## Expected Result

The LCD shows the clock and fills in the weather data once the network requests succeed.

## Troubleshooting

If time or weather does not update, check the Wi-Fi connection and internet access. Use local values for `wifi_ssid` and `wifi_password`.
