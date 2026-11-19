# Board diagnostics and RGB animation

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Runs the LVGL board status panel, scans for wireless devices and animates the onboard RGB LED.

## Hardware Required

- ESP32-C6-LCD-1.3 development board and USB data cable
- FAT32-formatted microSD card for the storage check (optional)

## How to Run

Open an ESP-IDF terminal in this directory using the version listed in the [repository README](../../../README.md).

```bash
idf.py set-target esp32c6
idf.py build flash monitor
```

## Expected Result

The LCD shows board information and the RGB LED cycles through colors.

## Troubleshooting

If card capacity is unavailable, check that the card is inserted and formatted as FAT32.
