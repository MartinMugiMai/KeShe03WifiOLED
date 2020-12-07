#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <SPI.h>
#include <U8g2lib.h>

//U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 0, /* data=*/ 4, /* cs=*/ 15, /* dc=*/ 16, /* reset=*/ 5);

//U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 4, /* reset=*/ 0);

//U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
//U8G2_SSD1306_128X32_UNIVISION_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE);   // ESP32 Thing, pure SW emulated I2C

HTTPClient http; //实例化一个httpclinet请求类 C++ 写法
String nnowWeather = "";
String nowTemp = "";
int test21 = 21;


void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println();
  u8g2.begin();   //选择U8G2模式，或者U8X8模式
  u8g2.enableUTF8Print();
  get_WIFI();
  delay(5000);
  //httpWeather();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  
  httpWeather();
//  u8g2.firstPage();
//  do {
//    u8g2.setFont(u8g2_font_ncenB14_tr);
//    u8g2.drawStr(0,15,"Temp:");
//    u8g2.print(nowTemp);
//    
//  } while ( u8g2.nextPage() );
//  delay(1000);
  
  delay(5000);
}

void get_WIFI(){
  WiFi.mode(WIFI_STA);
  WiFi.begin("Smhot","ea6662336");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
    }
  Serial.println();
  Serial.print("Connected, IP 地址:");
  Serial.println(WiFi.localIP());

}

void httpWeather(){
  if (WiFi.status() == WL_CONNECTED){
    http.begin("http://api.seniverse.com/v3/weather/now.json?key=Sehu5Cll8Qd7jX5_Z&location=jiangmen&language=zh-Hans&unit=c");
    int httpCode = http.GET();
    Serial.println("httpCode:");
    Serial.println(httpCode);
    Serial.println(HTTP_CODE_OK);
    if (httpCode > 0 ){
      Serial.println(httpCode);
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY){
        Serial.println("that is OK");
        String respone = http.getString();
        Serial.println(respone);
        
        const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 210;
        DynamicJsonDocument doc(capacity);

        const char* json = "{\"results\":[{\"location\":{\"id\":\"WX4FBXXFKE4F\",\"name\":\"北京\",\"country\":\"CN\",\"path\":\"北京,北京,中国\",\"timezone\":\"Asia/Shanghai\",\"timezone_offset\":\"+08:00\"},\"now\":{\"text\":\"晴\",\"code\":\"0\",\"temperature\":\"5\"},\"last_update\":\"2020-11-23T15:20:00+08:00\"}]}";

        deserializeJson(doc, respone);

        JsonObject results_0 = doc["results"][0];

        JsonObject results_0_location = results_0["location"];
        const char* results_0_location_id = results_0_location["id"]; // "WX4FBXXFKE4F"
        const char* results_0_location_name = results_0_location["name"]; // "北京"
        const char* results_0_location_country = results_0_location["country"]; // "CN"
        const char* results_0_location_path = results_0_location["path"]; // "北京,北京,中国"
        const char* results_0_location_timezone = results_0_location["timezone"]; // "Asia/Shanghai"
        const char* results_0_location_timezone_offset = results_0_location["timezone_offset"]; // "+08:00"

        JsonObject results_0_now = results_0["now"];
        const char* results_0_now_text = results_0_now["text"]; // "晴"
        const char* results_0_now_code = results_0_now["code"]; // "0"
        const char* results_0_now_temperature = results_0_now["temperature"]; // "5"

        const char* results_0_last_update = results_0["last_update"]; // "2020-11-23T15:20:00+08:00"

        nnowWeather = results_0_now_text;
        nowTemp = results_0_now_temperature;
        

        http.end();
        Serial.println("江门天气是:");
        Serial.println(nnowWeather);
        Serial.println("温度:");
        Serial.println(nowTemp);
        //u8g2.setFont(u8g2_font_unifont_t_chinese2);
        u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
        u8g2.setFontDirection(0);
        u8g2.firstPage();
          do {
            //u8g2.setFont(u8g2_font_wqy16_t_gb2312a);
            // u8g2.setFont(u8g2_font_unifont_t_chinese2);
            // u8g2.setFontDirection(0);
            u8g2.setCursor(10, 10);
            u8g2.print("江门天气:");
            u8g2.print(nnowWeather);
            u8g2.setCursor(0, 25);
            u8g2.print("温度:");
            //u8g2.drawStr(0, 35, "温度:");

            u8g2.setCursor(40,25);
            u8g2.print(nowTemp);
            
          } while ( u8g2.nextPage() );
          delay(1000);
        delay(100);
      }else {
        Serial.printf(http.errorToString(httpCode).c_str());

      }

      http.end();

    }else{
      Serial.println("unable to connect");
    }
  }
  delay(10000);
}
