# LVGL board diagnostics

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Shows Flash size, microSD card capacity and Wi-Fi/BLE scan counts in an LVGL interface. The example also exercises the onboard RGB LED.

## Hardware Required

- ESP32-C6-LCD-1.3 development board and USB data cable
- FAT32-formatted microSD card for the storage check (optional)

## How to Run

1. Install the Arduino-ESP32 version listed in the [repository README](../../../../README.md).
2. Set the sketchbook location to `examples/arduino/`, or copy the [bundled libraries](../../libraries/) into your sketchbook's `libraries` directory.
3. Select **ESP32C6 Dev Module** and match the board options in the [product configuration](../../../../.github/product.json).
4. Open [`LVGL_Arduino.ino`](LVGL_Arduino.ino) and upload.

## Expected Result

The LCD shows the onboard status panel. Wireless device counts update after the scan.

## Troubleshooting

If card capacity is unavailable, make sure the card is inserted and formatted as FAT32.
