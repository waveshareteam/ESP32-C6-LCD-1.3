# microSD 视频和图片播放

[English](README.md) | [简体中文](README_ZH.md)

## 功能

开机测试屏幕和 RGB 灯的颜色，随后从 microSD 卡播放 MJPEG 视频或显示 PNG 图片。

## 硬件需求

- ESP32-C6-LCD-1.3 开发板和 USB 数据线
- 存有兼容媒体文件的 FAT32 格式 microSD 卡

## 怎么运行

1. 在开发板管理器中安装 [仓库首页](../../../../README_ZH.md) 所列版本的 Arduino-ESP32。
2. 将项目文件夹位置设为 `examples/arduino/`，或将 [随附的库](../../libraries/) 复制到项目文件夹的 `libraries` 目录。
3. 选择 **ESP32C6 Dev Module**，并按 [产品配置](../../../../.github/product.json) 设置「工具」菜单。
4. 插入存有兼容媒体的存储卡，也可以使用[官方示例包](https://docs.waveshare.net/ESP32-C6-LCD-1.3/Resources-And-Documents/)中的样本文件。打开 [`Video_demo.ino`](Video_demo.ino) 并上传。

## 预期现象

显示屏和板载 RGB 灯依次显示红、绿、蓝，之后显示卡内的视频或图片。

## 常见问题

视频文件应放在卡的根目录，命名为 `video1.mjpeg`、`video2.mjpeg` 或 `video3.mjpeg`。没有匹配的视频时，示例会查找同一目录下的 PNG 图片。
