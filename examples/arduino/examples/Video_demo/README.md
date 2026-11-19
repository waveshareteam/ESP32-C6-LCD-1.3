# microSD video and image playback

[English](README.md) | [简体中文](README_ZH.md)

## What It Does

Runs an RGB color check and then plays MJPEG video or shows PNG images from a microSD card.

## Hardware Required

- ESP32-C6-LCD-1.3 development board and USB data cable
- FAT32-formatted microSD card with compatible media files

## How to Run

1. Install the Arduino-ESP32 version listed in the [repository README](../../../../README.md).
2. Set the sketchbook location to `examples/arduino/`, or copy the [bundled libraries](../../libraries/) into your sketchbook's `libraries` directory.
3. Select **ESP32C6 Dev Module** and match the board options in the [product configuration](../../../../.github/product.json).
4. Insert a card with your own compatible media, or use the sample files from the [official demo package](https://docs.waveshare.com/ESP32-C6-LCD-1.3/Resources-And-Documents). Open [`Video_demo.ino`](Video_demo.ino) and upload.

## Expected Result

The display and onboard LED show red, green and blue in sequence. The display then shows video or images from the card.

## Troubleshooting

Put `video1.mjpeg`, `video2.mjpeg` or `video3.mjpeg` in the card root for video playback. If no matching clip is present, the sketch looks for PNG images in the same directory.
