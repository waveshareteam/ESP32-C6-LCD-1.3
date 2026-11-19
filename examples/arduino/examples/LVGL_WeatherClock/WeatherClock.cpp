#include "WeatherClock.h"
#include "LVGL_Driver.h"  // 包含LVGL_WIDTH和LVGL_HEIGHT定义

// WiFi配置
const char* wifi_ssid = "YOUR_WIFI_SSID";
const char* wifi_password = "YOUR_WIFI_PASSWORD";

// 城市代码
String cityCode = "101280503";
String follower = "我思故我在";

// NTP服务器配置
static const char ntpServerName[] = "time.nist.gov";
const int timeZone = 8;  // 东八区
WiFiUDP Udp;
unsigned int localPort = 8000;
WiFiClient wificlient;

// 时间同步参数
byte setNTPSyncTime = 20;  // 20分钟同步一次
time_t prevDisplay = 0;

// 天气更新参数
unsigned long weaterTime = 0;
unsigned long wdsdTime = 0;
byte wdsdValue = 0;
String wendu = "";
String shidu = "";

// 滚动文字
String scrollText[7];
int currentIndex = 0;
unsigned long prevTime = 0;

// LVGL对象
static lv_obj_t *time_hm_label = NULL;
static lv_obj_t *time_s_label = NULL;
static lv_obj_t *week_label = NULL;
static lv_obj_t *monthday_label = NULL;
static lv_obj_t *city_label = NULL;
static lv_obj_t *temp_label = NULL;
static lv_obj_t *humidity_label = NULL;
static lv_obj_t *air_label = NULL;
static lv_obj_t *battery_label = NULL;
static lv_obj_t *tip_label = NULL;
static lv_obj_t *wind_label = NULL;
static lv_obj_t *follower_label = NULL;
static lv_obj_t *now_label = NULL;
static lv_obj_t *gif_img = NULL;
static lv_obj_t *min_temp_label = NULL;  // 最低温度标签
static lv_obj_t *weather_icon_img = NULL;  // 天气图标
static lv_obj_t *temp_icon_img = NULL;  // 温度图标
static lv_obj_t *humidity_icon_img = NULL;  // 湿度图标
static lv_obj_t *line_top = NULL;  // 顶部线框
static lv_obj_t *line_mid = NULL;  // 中间线框
static lv_obj_t *line_bottom = NULL;  // 底部线框
static lv_obj_t *line_vertical = NULL;  // 垂直线框
static lv_obj_t *status_label = NULL;   // 连接状态提示
static lv_obj_t *hint_label = NULL;     // 热点信息提示

// 样式
static lv_style_t style_time_hm;
static lv_style_t style_time_s;
static lv_style_t style_week;
static lv_style_t style_monthday;
static lv_style_t style_city;
static lv_style_t style_temp;
static lv_style_t style_humidity;
static lv_style_t style_air;
static lv_style_t style_battery;
static lv_style_t style_tip;
static lv_style_t style_wind;
static lv_style_t style_follower;
static lv_style_t style_now;

// 背光控制
bool AutoBright = false;
unsigned long backlightTime = 0;
  
// 电池电量（模拟，实际需要读取电池ADC）
int batteryLevel = 69;

// FreeRTOS 任务相关
static TaskHandle_t weatherTaskHandle = NULL;
static SemaphoreHandle_t weatherRequestSem = NULL;
static bool weatherUpdateRequested = false;
static bool cityCodeInitialized = false;
static bool initialWeatherReady = false;
static bool clockScreenReady = false;
static bool wifiTimedOut = false;
static const lv_font_t *font_status = &lv_font_YZGJHeiFont_18;

// 状态页展示
static void showStatusScreen(const char *hintText, const char *statusText) {
  lv_obj_clean(lv_scr_act());
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(BG_COLOR), 0);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
  lv_obj_set_size(lv_scr_act(), LCD_WIDTH, LCD_HEIGHT);
  lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

  hint_label = lv_label_create(lv_scr_act());
  lv_label_set_text(hint_label, hintText);
  lv_obj_set_style_text_color(hint_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(hint_label, font_status, 0);
  lv_label_set_long_mode(hint_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(hint_label, LCD_WIDTH - 20);
  lv_obj_set_pos(hint_label, 10, 30);

  status_label = lv_label_create(lv_scr_act());
  lv_label_set_text(status_label, statusText);
  lv_obj_set_style_text_color(status_label, lv_color_white(), 0);
  lv_obj_set_style_text_font(status_label, font_status, 0);
  lv_label_set_long_mode(status_label, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(status_label, LCD_WIDTH - 20);
  lv_obj_set_pos(status_label, 10, 120);
}

// HTTP请求函数
String httpGetRequest(String url, String userAgent, String referer) {
  String response = "";
  
  // 检查WiFi连接状态
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("HTTP请求失败：WiFi未连接");
    Serial.print("WiFi状态: ");
    Serial.println(WiFi.status());
    return "";
  }
  
  int protocolEnd = url.indexOf("://");
  if (protocolEnd == -1) {
    Serial.println("HTTP请求失败：URL格式错误");
    return "";
  }
  
  String protocol = url.substring(0, protocolEnd);
  String hostPath = url.substring(protocolEnd + 3);
  
  int hostEnd = hostPath.indexOf("/");
  if (hostEnd == -1) {
    hostEnd = hostPath.length();
    hostPath += "/";
  }
  String host = hostPath.substring(0, hostEnd);
  String path = hostPath.substring(hostEnd);
  
  Serial.print("正在连接服务器: ");
  Serial.print(host);
  Serial.print(":80 ... ");
  
  // 设置连接超时时间（缩短防止界面卡住）
  wificlient.setTimeout(3000);
  
  if (!wificlient.connect(host.c_str(), 80)) {
    Serial.println("失败");
    Serial.print("连接错误代码: ");
    Serial.println(wificlient.getWriteError());
    return "";
  }
  
  Serial.println("成功");
  
  wificlient.print("GET ");
  wificlient.print(path);
  wificlient.println(" HTTP/1.1");
  wificlient.print("Host: ");
  wificlient.println(host);
  if (userAgent.length() > 0) {
    wificlient.print("User-Agent: ");
    wificlient.println(userAgent);
  }
  if (referer.length() > 0) {
    wificlient.print("Referer: ");
    wificlient.println(referer);
  }
  wificlient.println("Connection: close");
  wificlient.println();
  
  unsigned long timeout = millis();
  bool headerEnd = false;
  while (wificlient.connected() && (millis() - timeout < 3000)) {  // 缩短到3秒
    if (wificlient.available()) {
      String line = wificlient.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) {
        headerEnd = true;
        break;
      }
    }
    lv_timer_handler(); // 保持界面刷新
    delay(1);
  }
  
  if (headerEnd) {
    timeout = millis();
    while (wificlient.connected() && (millis() - timeout < 3000)) {  // 缩短到3秒
      if (wificlient.available()) {
        response += wificlient.readString();
      }
      lv_timer_handler(); // 保持界面刷新
      delay(1);
    }
  }
  
  wificlient.stop();
  
  Serial.print("HTTP响应接收完成，长度: ");
  Serial.println(response.length());
  
  return response;
}

// 获取城市代码
void getCityCode(bool autoGetWeather) {
  // 检查WiFi连接状态
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("获取城市代码失败：WiFi未连接");
    Serial.print("WiFi状态: ");
    Serial.println(WiFi.status());
    return;
  }
  
  String URL = "http://wgeo.weather.com.cn/ip/?_=2" + String(now());
  Serial.print("Send GET request to URL: ");
  Serial.println(URL);
  
  String str = httpGetRequest(URL, 
    "Mozilla/5.0 (iPhone; CPU iPhone OS 11_0 like Mac OS X) AppleWebKit/604.1.38 (KHTML, like Gecko) Version/11.0 Mobile/15A372 Safari/604.1",
    "http://www.weather.com.cn/");
    
  Serial.print("响应长度: ");
  Serial.println(str.length());
  
  if (str.length() > 0) {
    Serial.print("响应内容（前200字符）: ");
    Serial.println(str.substring(0, 200));
    
    int aa = str.indexOf("id=");
    if(aa > -1) {
      cityCode = str.substring(aa+4, aa+4+9);
      Serial.print("获取到的城市代码: ");
      Serial.println(cityCode);
      cityCodeInitialized = true;
      if (autoGetWeather) {
        getCityWeater();
      }
    } else {
      Serial.println("获取城市代码失败：响应中未找到id=");
      Serial.println("尝试使用默认城市代码");
      // 如果获取失败，使用默认城市代码
      cityCode = "101280503";  // 深圳
      cityCodeInitialized = true;
      if (autoGetWeather) {
        getCityWeater();
      }
    }
  } else {
    Serial.println("请求城市代码错误：连接失败或响应为空");
    Serial.println("尝试使用默认城市代码");
    // 如果连接失败，使用默认城市代码
    cityCode = "101280503";  // 深圳
    cityCodeInitialized = true;
    if (autoGetWeather) {
      getCityWeater();
    }
  }
}

// 获取城市天气
void getCityWeater() {
  String URL = "http://d1.weather.com.cn/weather_index/" + cityCode + ".html?_=" + String(now());
  
  Serial.println("正在获取天气数据");
  Serial.println(URL);
  initialWeatherReady = false;
  String str = httpGetRequest(URL,
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/116.0.0.0 Safari/537.36 Edg/116.0.1938.76",
    "http://www.weather.com.cn/");
  if (str.length() > 0) {
    int indexStart = str.indexOf("weatherinfo\":");
    int indexEnd = str.indexOf("};var alarmDZ");
    String jsonCityDZ = str.substring(indexStart + 13, indexEnd);
    
    indexStart = str.indexOf("dataSK =");
    indexEnd = str.indexOf(";var dataZS");
    String jsonDataSK = str.substring(indexStart + 8, indexEnd);
    
    indexStart = str.indexOf("\"f\":[");
    indexEnd = str.indexOf(",{\"fa");
    String jsonFC = str.substring(indexStart + 5, indexEnd);
    
    weaterData(&jsonCityDZ, &jsonDataSK, &jsonFC);
    Serial.println("获取成功");
    initialWeatherReady = true;
  } else {
    Serial.println("请求城市天气错误：连接失败");
    initialWeatherReady = false;
  }
}

// 解析天气数据
void weaterData(String *cityDZ, String *dataSK, String *dataFC) {
  DynamicJsonDocument doc(4096);
  deserializeJson(doc, *dataSK);
  JsonObject sk = doc.as<JsonObject>();
  
  // 温度
  wendu = sk["temp"].as<String>();
  
  // 湿度
  shidu = sk["SD"].as<String>();
  
  // 城市名称
  if (city_label) {
    lv_label_set_text(city_label, sk["cityname"].as<String>().c_str());
  }
  
  // PM2.5空气指数（使用与ESP32S3相同的分级）
  String aqiTxt = "优";
  int pm25V = sk["aqi"].as<int>();
  if (pm25V > 200) {
    aqiTxt = "重度";
  } else if (pm25V > 150) {
    aqiTxt = "中度";
  } else if (pm25V > 100) {
    aqiTxt = "轻度";
  } else if (pm25V > 50) {
    aqiTxt = "良";
  }
  
  if (air_label) {
    lv_label_set_text(air_label, aqiTxt.c_str());
  }
  
  // 温度显示
  if (temp_label) {
    String tempStr = wendu + "℃";
    lv_label_set_text(temp_label, tempStr.c_str());
  }
  
  // 湿度显示
  if (humidity_label) {
    String humidityStr = shidu + "%";
    lv_label_set_text(humidity_label, humidityStr.c_str());
  }
  
  // 风向
  if (wind_label) {
    String windStr = sk["WD"].as<String>() + sk["WS"].as<String>();
    lv_label_set_text(wind_label, windStr.c_str());
  }
  
  scrollText[0] = "实时天气:" + sk["weather"].as<String>();
  scrollText[1] = "空气质量:" + aqiTxt;
  scrollText[2] = "风向:" + sk["WD"].as<String>() + sk["WS"].as<String>();

  // 更新天气图标
  int weatherCode = atoi((sk["weathercode"].as<String>()).substring(1, 3).c_str());
  updateWeatherIcon(weatherCode);

  // 解析第二段JSON
  deserializeJson(doc, *cityDZ);
  JsonObject dz = doc.as<JsonObject>();
  scrollText[3] = "今日:" + dz["weather"].as<String>();
  
  deserializeJson(doc, *dataFC);
  JsonObject fc = doc.as<JsonObject>();
  // 尝试使用℃符号，如果字体不支持会显示为空白
  scrollText[4] = "最低温度:" + fc["fd"].as<String>() + "℃";
  scrollText[5] = "最高温度:" + fc["fc"].as<String>() + "℃";
  
  // 更新最低温度显示
  if (min_temp_label) {
    String minTempStr = "Min: " + fc["fd"].as<String>() + "℃";
    // 如果有中文字体，可以使用：String minTempStr = "最低温度 " + fc["fd"].as<String>() + "℃";
    lv_label_set_text(min_temp_label, minTempStr.c_str());
  }
}



// 更新天气图标（根据天气代码，使用JPG数据）
// 注意：需要包含所有天气图标的头文件，例如：
// #include "img/tianqi/t0.h"
// #include "img/tianqi/t1.h"
// ... 等等
void updateWeatherIcon(int weatherCode) {
  if (weather_icon_img == NULL) return;

  LV_UNUSED(weatherCode);
}

// 更新温湿度图标显示（使用JPG数据）
void updateTempHumidityIcon(bool showTemp) {
  LV_UNUSED(showTemp);
}

// 温湿度同时显示（不再轮播切换）
void weatherWarning() {
  // 同时显示温度和湿度，不再切换
  if (temp_label && wendu.length() > 0) {
    // 尝试使用℃符号，如果字体不支持会显示为空白，则使用°C或C
    // 新字体可能包含℉但不包含℃，先尝试℃
    String tempStr = wendu + "℃";  // 使用℃符号
    lv_label_set_text(temp_label, tempStr.c_str());
    lv_obj_clear_flag(temp_label, LV_OBJ_FLAG_HIDDEN);
  }
  
  if (humidity_label && shidu.length() > 0) {
    String humidityStr = shidu + "";
    lv_label_set_text(humidity_label, humidityStr.c_str());
    lv_obj_clear_flag(humidity_label, LV_OBJ_FLAG_HIDDEN);
  }
  
  // 显示温度图标
  if (temp_icon_img) {
    lv_obj_clear_flag(temp_icon_img, LV_OBJ_FLAG_HIDDEN);
  }
  
  // 显示湿度图标
  if (humidity_icon_img) {
    lv_obj_clear_flag(humidity_icon_img, LV_OBJ_FLAG_HIDDEN);
  }
}

// 滚动文字（参考代码中的scrollTxt函数，有滚动动画效果）
// 参考代码使用for循环实现从pos=20到pos=0的滚动效果
void scrollBanner() {
  if (millis() - prevTime > 1500) { // 缩短间隔，加快滚轮切换
    if (scrollText[currentIndex].length() > 0 && tip_label) {
      // 更新文字内容，确保使用UTF-8编码
      lv_label_set_text(tip_label, scrollText[currentIndex].c_str());
      // 强制刷新标签，确保文字显示
      lv_obj_invalidate(tip_label);
      
      // 实现滚动动画效果（从下往上，参考代码pos从20到0）
      // 使用LVGL动画实现平滑滚动
      lv_anim_t a;
      lv_anim_init(&a);
      lv_anim_set_var(&a, tip_label);
      lv_anim_set_values(&a, 232, 212);  // 与LVGL界面位置匹配的滚动
      lv_anim_set_time(&a, 300);  // 动画时间300ms
      lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
      lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
      lv_anim_start(&a);
      
      if (currentIndex >= 5) {
        currentIndex = 0;
      } else {
        currentIndex += 1;
      }
    }
    prevTime = millis();
  }
}

// 粉丝数显示
void fanspush() {
  if (follower_label) {
    lv_label_set_text(follower_label, follower.c_str());
  }
}

// WiFi连接
void get_wifi() {
  wifiTimedOut = false;
  if (status_label) {
    // lv_label_set_text(status_label, "正在连接WiFi 请打开手机热点\n热点名称：YOUR_WIFI_SSID\n密码：YOUR_WIFI_SSID123");
    lv_timer_handler(); // 立即刷新提示，防止等待时黑屏
  }
  Serial.print("Connecting to WiFi: ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    lv_timer_handler(); // 保持界面刷新
    
    if (millis() - startTime > 30000) {
      Serial.println("");
      Serial.println("WiFi connection timeout!");
      Serial.println("Continuing without WiFi...");
      wifiTimedOut = true;
      if (status_label) {
        lv_label_set_text(status_label, "WIFI连接超时，请检查WIFI配置后RESET设备");
        lv_timer_handler();
      }
      return;
    }
    yield();
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  if (status_label) {
    lv_label_set_text(status_label, "WiFi连接 正在获取城市温度湿度信息中。");
    lv_timer_handler();
  }
}

// NTP时间获取
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

time_t getNtpTime() {
  // 检查WiFi连接状态
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("NTP同步失败：WiFi未连接");
    return 0;
  }
  
  IPAddress ntpServerIP;
  
  // 清空UDP缓冲区
  while (Udp.parsePacket() > 0) {
    Udp.flush();
  }
  
  // 解析NTP服务器地址
  if (WiFi.hostByName(ntpServerName, ntpServerIP) == 0) {
    Serial.println("NTP同步失败：无法解析NTP服务器地址");
    return 0;
  }
  
  Serial.print("正在连接NTP服务器: ");
  Serial.print(ntpServerName);
  Serial.print(" (");
  Serial.print(ntpServerIP);
  Serial.println(")");
  
  sendNTPpacket(ntpServerIP);
  
  uint32_t beginWait = millis();
  while (millis() - beginWait < 3000) {  // 增加超时时间到3秒
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("NTP同步成功！");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      time_t timeValue = secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      Serial.print("获取到的NTP时间: ");
      Serial.println(timeValue);
      return timeValue;
    }
    delay(10);  // 短暂延迟，避免CPU占用过高
    lv_timer_handler();  // 保持界面响应
  }
  Serial.println("NTP同步失败：超时");
  return 0;
}

void sendNTPpacket(IPAddress &address) {
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

// 时间显示函数
// 辅助函数：将中文转换为英文（如果没有中文字体）
String convertChineseToEnglish(String chinese) {
  if (chinese == "现在") return "Now";
  if (chinese == "你好") return "Hi";
  if (chinese == "优") return "Good";
  if (chinese == "良") return "Fair";
  if (chinese == "轻度") return "Light";
  if (chinese == "中度") return "Mod";
  if (chinese == "重度") return "Heavy";
  return chinese;  // 如果找不到对应，返回原文本
}

String week() {
  // 新字体支持"二"，尝试使用所有中文数字
  // 如果字体不支持某些字符，LVGL会显示为空白或使用fallback字体
  int dayOfWeek = weekday() - 1;
  if (dayOfWeek < 0 || dayOfWeek >= 7) {
    dayOfWeek = 0;  // 防止数组越界
  }
  
  // 使用中文数字：周一、周二、周三、周四、周五、周六、周日
  String wk[7] = { "日", "一", "二", "三", "四", "五", "六" };
  String s = "周" + wk[dayOfWeek];
  
  return s;
}

String monthDay() {
  String s = String(month());
  s = s + "月" + day() + "日";
  return s;
}

String hourMinute() {
  String s = num2str(hour());
  s = s + ":" + num2str(minute());
  return s;
}

String num2str(int digits) {
  String s = "";
  if (digits < 10)
    s = s + "0";
  s = s + digits;
  return s;
}

void digitalClockDisplay() {
  // 检查时间是否有效（时间同步后 now() 会返回有效值）
  time_t currentTime = now();
  
  // 如果时间有效且发生变化，则更新显示
  if (currentTime > 0 && currentTime != prevDisplay) {
    prevDisplay = currentTime;

    // 更新时分秒
    if (time_hm_label) {
      String timeStr = num2str(hour()) + ":" + num2str(minute()) + ":" + num2str(second());
      lv_label_set_text(time_hm_label, timeStr.c_str());
      // 强制刷新标签
      lv_obj_invalidate(time_hm_label);
    }

    // 更新星期
    if (week_label) {
      lv_label_set_text(week_label, week().c_str());
      lv_obj_invalidate(week_label);
    }
    
    // 更新月日
    if (monthday_label) {
      lv_label_set_text(monthday_label, monthDay().c_str());
      lv_obj_invalidate(monthday_label);
    }
  } else if (currentTime == 0) {
    // 时间未同步，显示等待提示
    static unsigned long lastSyncCheck = 0;
    if (millis() - lastSyncCheck > 5000) {  // 每5秒检查一次
      lastSyncCheck = millis();
      Serial.println("等待NTP时间同步...");
      if (time_hm_label) {
        lv_label_set_text(time_hm_label, "同步中...");
        lv_obj_invalidate(time_hm_label);
      }
    }
  }
}

// 背光控制
void setBacklight(bool on) {
  // 根据实际硬件调整，这里假设低电平有效
  digitalWrite(EXAMPLE_PIN_NUM_BK_LIGHT, on ? LOW : HIGH);
  Serial.print("Backlight: ");
  Serial.println(on ? "ON" : "OFF");
}

void backlightControl() {
  if (AutoBright) {
    int currentHour = hour();
    if (currentHour >= 22 || currentHour < 6) {
      setBacklight(false);
    } else {
      setBacklight(true);
    }
  }
}

// 创建线框装饰（参考代码中的drawFastHLine等）
void createDecorativeLines(void) {
  // 与LVGL生成界面保持一致的线框布局
  line_top = lv_line_create(lv_scr_act());
  static lv_point_t line_top_points[] = {{0, 0}, {240, 0}};
  lv_line_set_points(line_top, line_top_points, 3);
  lv_obj_set_style_line_color(line_top, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_top, 2, 0);
  lv_obj_set_style_line_opa(line_top, LV_OPA_COVER, 0);

  static lv_obj_t *line_top2 = lv_line_create(lv_scr_act());
  static lv_point_t line_top2_points[] = {{0, 0}, {240, 0}};
  lv_line_set_points(line_top2, line_top2_points, 2);
  lv_obj_set_pos(line_top2, 0, 35);
  lv_obj_set_style_line_color(line_top2, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_top2, 2, 0);
  lv_obj_set_style_line_opa(line_top2, LV_OPA_COVER, 0);

  static lv_obj_t *line_vertical_top = lv_line_create(lv_scr_act());
  static lv_point_t line_vt_points[] = {{0, 0}, {0, 32}};  // 限制高度，不超出屏幕
  lv_line_set_points(line_vertical_top, line_vt_points, 2);
  lv_obj_set_pos(line_vertical_top, 100, 2);
  lv_obj_set_size(line_vertical_top, 1, 32);
  lv_obj_set_style_line_color(line_vertical_top, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_vertical_top, 2, 0);
  lv_obj_set_style_line_opa(line_vertical_top, LV_OPA_COVER, 0);

  line_bottom = lv_line_create(lv_scr_act());
  static lv_point_t line_bottom_points[] = {{0, 0}, {240, 0}};
  lv_line_set_points(line_bottom, line_bottom_points, 2);
  lv_obj_set_pos(line_bottom, 0, 171);
  lv_obj_set_style_line_color(line_bottom, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_bottom, 2, 0);
  lv_obj_set_style_line_opa(line_bottom, LV_OPA_COVER, 0);

  static lv_obj_t *line_bottom2 = lv_line_create(lv_scr_act());
  static lv_point_t line_bottom2_points[] = {{0, 0}, {240, 0}};
  lv_line_set_points(line_bottom2, line_bottom2_points, 2);
  lv_obj_set_pos(line_bottom2, 0, 205);
  lv_obj_set_style_line_color(line_bottom2, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_bottom2, 2, 0);
  lv_obj_set_style_line_opa(line_bottom2, LV_OPA_COVER, 0);

  // 删除底部第三条线（在238位置，可能超出屏幕或显示为灰色条纹）
  static lv_obj_t *line_bottom3 = lv_line_create(lv_scr_act());
  static lv_point_t line_bottom3_points[] = {{0, 0}, {240, 0}};
  lv_line_set_points(line_bottom3, line_bottom3_points, 2);
  lv_obj_set_pos(line_bottom3, 0, 238);
  lv_obj_set_style_line_color(line_bottom3, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_bottom3, 2, 0);
  lv_obj_set_style_line_opa(line_bottom3, LV_OPA_COVER, 0);

  static lv_obj_t *line_vertical1 = lv_line_create(lv_scr_act());
  static lv_point_t line_v1_points[] = {{0, 0}, {0, 34}};  // 限制高度，不超出屏幕
  lv_line_set_points(line_vertical1, line_v1_points, 2);
  lv_obj_set_pos(line_vertical1, 58, 171);
  lv_obj_set_size(line_vertical1, 1, 34);  // 限制大小
  lv_obj_set_style_line_color(line_vertical1, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_vertical1, 2, 0);
  lv_obj_set_style_line_opa(line_vertical1, LV_OPA_COVER, 0);

  static lv_obj_t *line_vertical2 = lv_line_create(lv_scr_act());
  static lv_point_t line_v2_points[] = {{0, 0}, {0, 34}};  // 限制高度，不超出屏幕
  lv_line_set_points(line_vertical2, line_v2_points, 2);
  lv_obj_set_pos(line_vertical2, 146, 171);
  lv_obj_set_size(line_vertical2, 1, 34);  // 限制大小
  lv_obj_set_style_line_color(line_vertical2, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_line_width(line_vertical2, 2, 0);
  lv_obj_set_style_line_opa(line_vertical2, LV_OPA_COVER, 0);
}

static void buildClockScreen(void) {
  lv_obj_clean(lv_scr_act()); // 进入页面前清屏
  // 清空屏幕
  // 设置屏幕背景色
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(BG_COLOR), 0);
  lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
  
  lv_obj_set_style_clip_corner(lv_scr_act(), true, 0);
  // 确保屏幕大小正确，并设置裁剪区域
  lv_obj_set_size(lv_scr_act(), LCD_WIDTH, LCD_HEIGHT);
  lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);  // 关闭滚动条
  lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);  // 禁用滚动
  


  // 创建装饰线框
  createDecorativeLines();

  // 字体与样式设置，优先使用LVGL工程中导出的字体
  const lv_font_t *font_large = LV_FONT_DEFAULT;
  const lv_font_t *font_mid = LV_FONT_DEFAULT;
  const lv_font_t *font_small = LV_FONT_DEFAULT;

  // 直接使用LVGL导出的字体，如果链接失败会退回到默认字体
  font_large = &lv_font_YZGJHeiFont_40;
  font_mid = &lv_font_YZGJHeiFont_18;
  font_small = &lv_font_YZGJHeiFont_18;

  lv_style_init(&style_time_hm);
  lv_style_set_text_color(&style_time_hm, lv_color_hex(TIME_HM_COLOR));
  lv_style_set_text_font(&style_time_hm, font_large);

  lv_style_init(&style_week);
  lv_style_set_text_color(&style_week, lv_color_hex(WEEK_COLOR));
  lv_style_set_text_font(&style_week, font_mid);

  lv_style_init(&style_monthday);
  lv_style_set_text_color(&style_monthday, lv_color_hex(MONTH_COLOR));
  lv_style_set_text_font(&style_monthday, font_mid);

  lv_style_init(&style_city);
  lv_style_set_text_color(&style_city, lv_color_hex(CITY_COLOR));
  lv_style_set_text_font(&style_city, font_mid);
  lv_style_set_text_align(&style_city, LV_TEXT_ALIGN_CENTER);

  lv_style_init(&style_temp);
  lv_style_set_text_color(&style_temp, lv_color_hex(TH_COLOR));
  lv_style_set_text_font(&style_temp, font_mid);

  lv_style_init(&style_humidity);
  lv_style_set_text_color(&style_humidity, lv_color_hex(TH_COLOR));
  lv_style_set_text_font(&style_humidity, font_mid);

  lv_style_init(&style_tip);
  lv_style_set_text_color(&style_tip, lv_color_hex(TIP_COLOR));
  lv_style_set_text_font(&style_tip, font_mid);  // 使用支持中文的字体
  lv_style_set_text_align(&style_tip, LV_TEXT_ALIGN_CENTER);

  lv_style_init(&style_follower);
  lv_style_set_text_color(&style_follower, lv_color_hex(0xFFFFFF));
  lv_style_set_text_font(&style_follower, font_mid);
  lv_style_set_text_align(&style_follower, LV_TEXT_ALIGN_CENTER);

  // 顶部城市
  city_label = lv_label_create(lv_scr_act());
  lv_label_set_text(city_label, "深圳");
  lv_obj_add_style(city_label, &style_city, 0);
  lv_obj_set_pos(city_label, 4, 5);
  lv_obj_set_size(city_label, 91, 25);

  // 顶部右侧文字
  follower_label = lv_label_create(lv_scr_act());
  lv_label_set_text(follower_label, follower.c_str());
  lv_obj_add_style(follower_label, &style_follower, 0);
  lv_obj_set_pos(follower_label, 105, 6);
  lv_obj_set_size(follower_label, 129, 26);

  // 中间时间标签（包含时分秒）
  time_hm_label = lv_label_create(lv_scr_act());
  lv_label_set_text(time_hm_label, "00:00:00");
  lv_obj_add_style(time_hm_label, &style_time_hm, 0);
  lv_obj_set_pos(time_hm_label, 34, 40);
  lv_obj_set_size(time_hm_label, 168, 51);
  lv_obj_set_style_text_align(time_hm_label, LV_TEXT_ALIGN_CENTER, 0);

  // 滚动字幕区域（左下红字）
  tip_label = lv_label_create(lv_scr_act());
  lv_label_set_text(tip_label, "实时天气");
  lv_obj_add_style(tip_label, &style_tip, 0);
  lv_obj_set_pos(tip_label, 31, 212);
  lv_obj_set_size(tip_label, 204, 20);
  // 确保使用支持中文的字体（font_mid已经在style_tip中设置，这里再次确认）
  lv_obj_set_style_text_font(tip_label, font_mid, 0);
  // 设置文本长文本模式，确保中文正确显示
  lv_label_set_long_mode(tip_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  // 设置文本溢出处理，防止显示超出边界
  lv_obj_set_style_clip_corner(tip_label, true, 0);
  // 禁用滚动，防止出现滚动提示
  lv_obj_clear_flag(tip_label, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scrollbar_mode(tip_label, LV_SCROLLBAR_MODE_OFF);
  // 设置文本长文本模式，确保中文正确显示
  lv_label_set_long_mode(tip_label, LV_LABEL_LONG_SCROLL_CIRCULAR);

  // 温湿度
  humidity_icon_img = lv_img_create(lv_scr_act());
  lv_img_set_src(humidity_icon_img, &_shidu_alpha_19x19);
  lv_obj_set_pos(humidity_icon_img, 5, 146);

  humidity_label = lv_label_create(lv_scr_act());
  lv_label_set_text(humidity_label, "--");
  lv_obj_add_style(humidity_label, &style_humidity, 0);
  lv_obj_set_pos(humidity_label, 24, 146);
  lv_obj_set_size(humidity_label, 51, 17);

  temp_icon_img = lv_img_create(lv_scr_act());
  lv_img_set_src(temp_icon_img, &_wendu_alpha_19x19);
  lv_obj_set_pos(temp_icon_img, 150, 178);

  temp_label = lv_label_create(lv_scr_act());
  lv_label_set_text(temp_label, "--℃");
  lv_obj_add_style(temp_label, &style_temp, 0);
  lv_obj_set_pos(temp_label, 169, 178);
  lv_obj_set_size(temp_label, 67, 18);

  // 天气大图标
  weather_icon_img = lv_img_create(lv_scr_act());
  lv_obj_set_pos(weather_icon_img, 3, 130);
  lv_obj_set_size(weather_icon_img, 64, 64);

  // 底部星期/月日
  week_label = lv_label_create(lv_scr_act());
  lv_label_set_text(week_label, "周一");
  lv_obj_add_style(week_label, &style_week, 0);
  lv_obj_set_pos(week_label, 5, 178);
  lv_obj_set_size(week_label, 53, 17);

  monthday_label = lv_label_create(lv_scr_act());
  lv_label_set_text(monthday_label, "1月1日");
  lv_obj_add_style(monthday_label, &style_monthday, 0);
  lv_obj_set_pos(monthday_label, 59, 178);
  lv_obj_set_size(monthday_label, 84, 19);

  // 底部品牌/电池占位（已删除，不再显示）
  // battery_label = lv_label_create(lv_scr_act());
  // lv_label_set_text(battery_label, "69%");
  // lv_obj_add_style(battery_label, &style_week, 0);
  // lv_obj_set_pos(battery_label, 200, 180);
  battery_label = NULL;  // 设置为NULL，不再显示

  // WaveShare logo
  lv_obj_t *logo = lv_img_create(lv_scr_act());
  lv_img_set_src(logo, &_WaveShare_alpha_25x25);
  lv_obj_set_pos(logo, 0, 209);

  // 太空人动画帧
  gif_img = lv_animimg_create(lv_scr_act());
  if (gif_img != NULL && screen_GIF_Demo1_imgs != NULL) {
    lv_obj_set_pos(gif_img, 83, 87);
    lv_obj_set_size(gif_img, 75, 72);
    lv_animimg_set_src(gif_img, (const void **)screen_GIF_Demo1_imgs, 20);
    lv_animimg_set_duration(gif_img, 40 * 20);  // 每帧40ms，共20帧，总时长800ms
    lv_animimg_set_repeat_count(gif_img, LV_ANIM_REPEAT_INFINITE);
    
    // 确保动画对象可见
    lv_obj_clear_flag(gif_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_opa(gif_img, LV_OPA_COVER, 0);
    
    // 启动动画
    lv_animimg_start(gif_img);
    
    // 强制刷新一次，确保动画显示
    lv_refr_now(NULL);
    
    Serial.println("GIF动画已启动");
    Serial.print("动画对象地址: ");
    Serial.println((uint32_t)gif_img, HEX);
    Serial.print("图像数组地址: ");
    Serial.println((uint32_t)screen_GIF_Demo1_imgs, HEX);
  } else {
    Serial.println("错误：GIF动画对象或图像数组为空");
    Serial.print("gif_img: ");
    Serial.println((uint32_t)gif_img, HEX);
    Serial.print("screen_GIF_Demo1_imgs: ");
    Serial.println((uint32_t)screen_GIF_Demo1_imgs, HEX);
  }

  // 初始化滚动文字数组
  for (int i = 0; i < 7; i++) {
    scrollText[i] = "";
  }
  // 提供占位，进入主界面后立即有首条文字
  scrollText[0] = "实时天气获取中...";
  currentIndex = 0;
  prevTime = millis() - 2000; // 让scrollBanner首次立即触发
  scrollBanner(); // 先显示一条，避免等待
  currentIndex = 0;
  prevTime = 0;
  clockScreenReady = true;
}

// 初始化天气时钟界面
void WeatherClock_Init(void) {
  clockScreenReady = false;
  cityCodeInitialized = false;
  initialWeatherReady = false;

  showStatusScreen(
    "打开手机热点  热点配置信息：\n热点名称：YOUR_WIFI_SSID\n密码：YOUR_WIFI_SSID123",
    "设备正在连接WIFI和获取城市温度湿度信息中"
  );
  lv_timer_handler();  // 立即刷新提示页，避免等待时黑屏

  get_wifi();
  if (WiFi.status() != WL_CONNECTED) {
    if (!wifiTimedOut && status_label) {
      lv_label_set_text(status_label, "WiFi未连接 请打开手机热点\n热点名称：YOUR_WIFI_SSID  密码：YOUR_WIFI_SSID123");
    }
    return;
  }

  lv_label_set_text(status_label, "设备正在连接WIFI和获取城市温度湿度信息中。");
  getCityCode(); // 内部会获取天气

  if (!initialWeatherReady) {
    // 天气初次获取失败，提示并继续进入主界面，使用占位文本等待后台重试
    lv_label_set_text(status_label, "获取城市温度湿度信息失败，将使用占位数据并重试");
    scrollText[0] = "实时天气加载失败，稍后重试";
    scrollText[1] = "请检查网络或热点";
    scrollText[2] = "使用默认城市代码";
    currentIndex = 0;
    prevTime = millis() - 2000; // 让滚轮尽快开始
  }

  // 初始化UDP用于NTP
  Udp.begin(localPort);

  // 创建天气更新后台任务（避免阻塞主循环）
  weatherRequestSem = xSemaphoreCreateBinary();
  if (weatherRequestSem != NULL) {
    xTaskCreatePinnedToCore(
      weatherTask,           // 任务函数
      "WeatherTask",         // 任务名称
      8192,                  // 堆栈大小
      NULL,                  // 参数
      1,                     // 优先级
      &weatherTaskHandle,    // 任务句柄
      1                      // 核心ID（1号核心）
    );
    Serial.println("天气更新后台任务已创建");
  } else {
    Serial.println("警告：无法创建天气更新信号量");
  }
  
  // 设置NTP时间同步
  Serial.println("开始NTP时间同步...");
  setSyncProvider(getNtpTime);
  setSyncInterval(setNTPSyncTime * 60);
  
  // 立即尝试同步时间（不等待自动同步）
  time_t syncTime = getNtpTime();
  if (syncTime > 0) {
    setTime(syncTime);
    Serial.print("时间已设置为: ");
    Serial.println(syncTime);
  } else {
    Serial.println("警告：首次NTP同步失败，将在后台重试");
  }

  // 构建主界面
  buildClockScreen();

  // 首次获取天气后的滚动文字等更新
  fanspush();
  // 如果首次天气失败，后台任务仍会重试
  if (weatherRequestSem != NULL) {
    requestWeatherUpdate();
  }
}

// 更新电池电量（已禁用，不再显示）
void updateBatteryLevel() {
  // 电池显示已删除，此函数不再执行任何操作
}

// 请求天气更新（由主循环调用，触发后台任务）
void requestWeatherUpdate(void) {
  if (weatherRequestSem != NULL) {
    xSemaphoreGive(weatherRequestSem);
  }
}

// 天气更新后台任务（FreeRTOS）
void weatherTask(void *parameter) {
  while (1) {
    // 等待信号量触发
    if (xSemaphoreTake(weatherRequestSem, portMAX_DELAY) == pdTRUE) {
      if (WiFi.status() == WL_CONNECTED) {
        // 首次需要先获取城市代码
        if (!cityCodeInitialized) {
          Serial.println("后台任务：首次获取城市代码");
          getCityCode(false); // 不自动获取天气，由任务控制
          // 获取城市代码后立即获取天气
          if (cityCodeInitialized) {
            Serial.println("后台任务：开始获取天气数据");
            getCityWeater();
          }
        } else {
          // 后续只获取天气
          Serial.println("后台任务：开始获取天气数据");
          getCityWeater();
        }
        Serial.println("后台任务：天气数据获取完成");
      } else {
        Serial.println("后台任务：WiFi未连接，跳过天气更新");
      }
    }
    // 短暂延迟，避免CPU占用过高
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// 更新天气时钟
void WeatherClock_Update(void) {
  if (!clockScreenReady) {
    return;
  }
  digitalClockDisplay();
  
  // 恢复周期性天气刷新，通过后台任务执行（5分钟更新一次）
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - weaterTime > 300000) { // 5分钟更新一次天气
      weaterTime = millis();
      requestWeatherUpdate(); // 触发后台任务，不阻塞主循环
    }
  }
  
  weatherWarning();
  scrollBanner();
  updateBatteryLevel();
  
  if (millis() - backlightTime > 60000) {
    backlightTime = millis();
    backlightControl();
  }
}

// 主循环
void WeatherClock_Loop(void) {
  WeatherClock_Update();
  // 移除delay，让lv_timer_handler能够更频繁地调用，确保GIF动画和时钟更新
}

