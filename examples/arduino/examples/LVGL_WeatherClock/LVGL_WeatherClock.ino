#include "SD_Card.h"
#include "Display_ST7789.h"
#include "LVGL_Driver.h"
#include "WeatherClock.h"

void setup()
{       
  Serial.begin(115200);
  delay(100);
  Serial.println("System starting...");
  
  Flash_test();
  LCD_Init();
  Lvgl_Init();
  SD_Init();   
  Set_Backlight(100);
  // 初始化天气时钟
  WeatherClock_Init();
  
  Serial.println("Setup complete!");
}

void loop()
{
  Timer_Loop();  // 调用 lv_timer_handler()，必须频繁调用以更新动画和定时器
  WeatherClock_Loop();
  // 移除delay，确保lv_timer_handler能够频繁调用，保证GIF动画和时钟正常工作
  // 如果CPU占用过高，可以添加短暂延迟，但不要超过5ms
  delay(1);  // 最小延迟，避免CPU占用过高
}
