# LVGL 天气时钟

[English](README.md) | [简体中文](README_ZH.md)

## 功能

在 ST7789 LCD 上显示时间和天气。示例通过 Wi-Fi 同步时间、获取天气数据。

随附的 [Arduino Time 库](../../libraries/Time/)采用 [GNU LGPL 2.1 或更高版本](../../libraries/Time/LICENSE)授权。

## 硬件需求

- ESP32-C6-LCD-1.3 开发板和 USB 数据线
- 可联网的 2.4 GHz Wi-Fi 网络

## 怎么运行

1. 在开发板管理器中安装 [仓库首页](../../../../README_ZH.md) 所列版本的 Arduino-ESP32。
2. 将项目文件夹位置设为 `examples/arduino/`，或将 [随附的库](../../libraries/) 复制到项目文件夹的 `libraries` 目录。
3. 选择 **ESP32C6 Dev Module**，并按 [产品配置](../../../../.github/product.json) 设置「工具」菜单。
4. 上传前在 [`WeatherClock.cpp`](WeatherClock.cpp) 中填写 Wi-Fi 名称和密码。打开 [`LVGL_WeatherClock.ino`](LVGL_WeatherClock.ino) 并上传。

## 预期现象

LCD 显示时钟，联网请求成功后更新天气数据。

## 常见问题

如果时间或天气没有更新，请检查 Wi-Fi 和互联网连接，并确认 `wifi_ssid` 和 `wifi_password` 已使用本地网络信息。
