# LVGL 板载外设检测

[English](README.md) | [简体中文](README_ZH.md)

## 功能

通过 LVGL 界面显示 Flash 容量、microSD 卡容量和 Wi-Fi/BLE 扫描数量，并测试板载 RGB 灯。

## 硬件需求

- ESP32-C6-LCD-1.3 开发板和 USB 数据线
- 用于检查存储功能的 FAT32 格式 microSD 卡（可选）

## 怎么运行

1. 在开发板管理器中安装 [仓库首页](../../../../README_ZH.md) 所列版本的 Arduino-ESP32。
2. 将项目文件夹位置设为 `examples/arduino/`，或将 [随附的库](../../libraries/) 复制到项目文件夹的 `libraries` 目录。
3. 选择 **ESP32C6 Dev Module**，并按 [产品配置](../../../../.github/product.json) 设置「工具」菜单。
4. 打开 [`LVGL_Arduino.ino`](LVGL_Arduino.ino) 并上传。

## 预期现象

LCD 显示板载状态面板；无线扫描结束后更新设备数量。

## 常见问题

如果没有显示存储卡容量，请检查卡是否插好，以及格式是否为 FAT32。
