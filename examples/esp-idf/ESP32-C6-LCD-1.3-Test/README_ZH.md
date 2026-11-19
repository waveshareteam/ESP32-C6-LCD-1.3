# 板载外设检测和 RGB 动画

[English](README.md) | [简体中文](README_ZH.md)

## 功能

显示 LVGL 板载状态面板，扫描无线设备，并演示板载 RGB 灯动画。

## 硬件需求

- ESP32-C6-LCD-1.3 开发板和 USB 数据线
- 用于检查存储功能的 FAT32 格式 microSD 卡（可选）

## 怎么运行

使用 [仓库首页](../../../README_ZH.md) 所列版本的 ESP-IDF，在本目录打开 ESP-IDF 终端。

```bash
idf.py set-target esp32c6
idf.py build flash monitor
```

## 预期现象

LCD 显示板载信息，RGB 灯循环切换颜色。

## 常见问题

如果没有显示存储卡容量，请检查卡是否插好，以及格式是否为 FAT32。
