#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
//#include <stdio.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <SoftwareSerial.h> //启用软串口用作语音播报串口通信

#include <iostream>
#include <string>
using namespace std;

//#include <Syn6288.h>

#define DHTPIN 2 //D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); //实例化DHT C++风格

SoftwareSerial softSerial1(14,12); //实例化软串口 rx使用D5 io14 与syn6288 tx相连 、 tx使用D6 io12引脚与syn6288 rx相连
unsigned int numDic[] = {0xD2, 0xBB, 0xB6, 0xFE, 0xC8, 0xFD, 0xCB, 0xC4, 0xCE, 0xE5, 0xC1, 0xF9, 0xC6, 0xDF, 0xB0, 0xCB, 0xBE, 0xC5, 0xCA, 0xAE};//2n-1 和2n-2
unsigned int numZero[] = {0xC1, 0xE3};
unsigned int text1[] = {0xCF, 0xD6, 0xD4, 0xDA, 0xBD, 0xAD, 0xC3, 0xC5, 0xCE, 0xC2, 0xB6, 0xC8, 0xCA, 0xC7}; //现在江门温度是
unsigned int text2[] = {0xFD, 0x00, 0x05, 0x01, 0x00, 0xB6, 0xC8, 0x87}; //度
unsigned int text3[] = {0xFD, 0x00, 0x11, 0x01, 0x00}; //初始5位
unsigned int text4[] = {0xFD, 0x00, 0x11, 0x01, 0x00, 0xCF, 0xD6, 0xD4, 0xDA, 0xBD, 0xAD, 0xC3, 0xC5, 0xCE, 0xC2, 0xB6, 0xC8, 0xCA, 0xC7, 0x92};
unsigned int textN2[] = {0xCF, 0xD6, 0xD4, 0xDA, 0xBD, 0xAD, 0xC3, 0xC5, 0xCE, 0xC2, 0xB6, 0xC8, 0xCA, 0xC7};
unsigned int textN1[] = {0xB6, 0xC8};

unsigned char text9wei[] = {0xFD, 0x00, 0x19, 0x01, 0x00, 0xCF, 0xD6, 0xD4, 0xDA, 0xBD, 0xAD, 0xC3, 0xC5, 0xCE, 0xC2, 0xB6, 0xC8, 0xCA, 0xC7, 0xC1, 0xF9, 0xC1, 0xF9, 0xC1, 0xF9, 0xB6, 0xC8, 0xDD};//19 20 21 22 23 24
unsigned int text7wei[] = {0xFD, 0x00, 0x07, 0x01, 0x00, 0xB6, 0xFE, 0xCA, 0xAE, 0xD7};
unsigned int text5wei[] = {0xFD, 0x00, 0x07, 0x01, 0x00, 0xB6, 0xFE, 0xB1};

unsigned int textt;

unsigned int cloudyDic[] = {0xD2, 0xf5, 0xCC, 0xEc};
//U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 0, /* data=*/ 4, /* cs=*/ 15, /* dc=*/ 16, /* reset=*/ 5);

//U8G2_SSD1306_128X64_NONAME_1_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 5, /* dc=*/ 4, /* reset=*/ 0);

//U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
//U8G2_SSD1306_128X32_UNIVISION_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE);   // Adafruit Feather ESP8266/32u4 Boards + FeatherWing OLED
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE);   // 参照ESP32 Thing, pure SW emulated I2C esp8266引脚 D1给SDL D2给CLK

HTTPClient http; //实例化一个httpclinet请求类 C++ 写法
String nnowWeather = "";
String nowTemp = "";
int test21 = 21;
float dhtH = 1.1;
float dhtT = 1.1;
int nowTempInt = 23;
int nWCode = 0;
int length1 = 0;
int zongduanState = 0;
int oledState = 1; //1显示温度 -1显示对比

//创建一个叫温度对比的C++类
class wenDuDuiBi
{
  public:
  //String dx = "";
  float chaZhi = 1.0;
  int zhuangtai = 0;
  void biYiXia(int webTemp, float hereTemp)
  {
    double dwebTemp = webTemp;
    if (dwebTemp < hereTemp)
      {
        //dx = "小于";
        chaZhi = hereTemp - dwebTemp;
        zhuangtai = 1;
      }
    else if (dwebTemp == hereTemp)
      {
        //dx = "等于";
        chaZhi = 0;
        zhuangtai = 0;
      }else if(dwebTemp > hereTemp){
        //dx = "大于";
        chaZhi = dwebTemp - hereTemp;
        zhuangtai = 2;
      }
  }
};

wenDuDuiBi wddb; //然后马上实例化一个温度对比

void setup(){
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  pinMode(13, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
  attachInterrupt(13, lowInterrupt, FALLING); //按键中断用D7 13 D8 15
  attachInterrupt(0, D3Interrupt, FALLING);
  softSerial1.begin(9600);
  softSerial1.listen();
  dht.begin();

  Serial.println();
  u8g2.begin();   //选择U8G2模式，或者U8X8模式
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
  u8g2.setFontDirection(0);
  get_WIFI();
  attachInterrupt(13, lowInterrupt, FALLING);
  delay(5000);
  httpWeather();
  //httpWeather();
  
}

ICACHE_RAM_ATTR void lowInterrupt(){
  //led = -led;
  Serial.println("66666666");
  //speechTemp(22);
  zongduanState = 1;
  detachInterrupt(13);
  attachInterrupt(13, lowInterrupt, FALLING);
  detachInterrupt(0);
  attachInterrupt(0, D3Interrupt, FALLING);
  
  
}
ICACHE_RAM_ATTR void D3Interrupt(){
  //led = -led;
  Serial.println("D3D3D3");
  //speechTemp(22);
  oledState = -(oledState);
  Serial.println("当前显示状态");
  Serial.println(oledState);
  detachInterrupt(13);
  attachInterrupt(13, lowInterrupt, FALLING);
  detachInterrupt(0);
  attachInterrupt(0, D3Interrupt, FALLING);
  
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //speechJM();
  get_DHT();//调用传感器函数
  if (oledState == 1)
  {
    u8g2.firstPage();
    do{
    u8g2.setCursor(10, 10);
    u8g2.print("江门天气:");
    u8g2.print(nnowWeather);
    u8g2.setCursor(0, 25);
    u8g2.print("温度:");
              //u8g2.drawStr(0, 35, "温度:");

    u8g2.setCursor(40,25);
    u8g2.print(nowTemp);
    u8g2.setCursor(0,40);
    u8g2.print("室内温度:");
    u8g2.setCursor(50,40);
    u8g2.print(dhtT);
    u8g2.setCursor(0,55);
    u8g2.print("室内湿度:");
    u8g2.setCursor(50,55);
    u8g2.print(dhtH);
    } while ( u8g2.nextPage() );
  }else if(oledState ==-1){
    u8g2.firstPage();
    do{
    u8g2.setCursor(10, 10);
    u8g2.print("江门天气:");
    u8g2.print(nnowWeather);
    u8g2.setCursor(0, 25);
    u8g2.print("气象台温度比室内温度");
    u8g2.setCursor(0,40);
    wddb.biYiXia(nowTempInt, dhtT);//调用温度对比
    // if (wddb.zhuangtai = 1)
    // {
    //   u8g2.print("低");
    //   u8g2.print(wddb.chaZhi);
    //   u8g2.print("摄氏度");
    // }
    switch (wddb.zhuangtai)
    {
    case 1:
      /* code */
      u8g2.print("低");
      u8g2.print(wddb.chaZhi);
      u8g2.print("摄氏度");
      break;
    case 0:
      u8g2.print("相等");
      // u8g2.print(wddb.chaZhi);
      // u8g2.print("摄氏度");
    case 2:
      u8g2.print("高");
      u8g2.print(wddb.chaZhi);
      u8g2.print("摄氏度");
    
    default:
      u8g2.print("没比较error");
      // u8g2.print(wddb.chaZhi);
      // u8g2.print("摄氏度");
      break;
    }

    } while ( u8g2.nextPage() );
  }
  
  // u8g2.firstPage();
  // do{
  // u8g2.setCursor(10, 10);
  // u8g2.print("江门天气:");
  // u8g2.print(nnowWeather);
  // u8g2.setCursor(0, 25);
  // u8g2.print("温度:");
  //           //u8g2.drawStr(0, 35, "温度:");

  // u8g2.setCursor(40,25);
  // u8g2.print(nowTemp);
  // u8g2.setCursor(0,40);
  // u8g2.print("室内温度:");
  // u8g2.setCursor(50,40);
  // u8g2.print(dhtT);
  // u8g2.setCursor(0,55);
  // u8g2.print("室内湿度:");
  // u8g2.setCursor(50,55);
  // u8g2.print(dhtH);
  // } while ( u8g2.nextPage() );

  String testt4 = "20";
  String qt = testt4;

  if (zongduanState == 1)
  {
    zongduanState = 0;
    Serial.println(zongduanState);
    //speechTemp(10);
    speechTemp(nowTempInt);
    delay(200);
    speechWea(nWCode);
    delay(500);
  }

  //speechTemp(qt.toInt());
  // length1 = sizeof(textN2) / sizeof(byte);
  // synout(textN2, length1);
  // length1 = 0;
  // length1 = sizeof(textN1) / sizeof(byte);
  // synout(textN1, length1);
  //speechJM2();
  //syn.play(text1, sizeof(text1), 1);
  // httpWeather();
  // if (zongduanState == 1)
  // {
  //   zongduanState = 0;
  //   Serial.println(zongduanState);
  //   speechTemp(22);
  //   delay(500);
  // }
  //  u8g2.firstPage();
  //  do {
  //    u8g2.setFont(u8g2_font_ncenB14_tr);
  //    u8g2.drawStr(0,15,"Temp:");
  //    u8g2.print(nowTemp);
  //
  //  } while ( u8g2.nextPage() );
  //  delay(1000);
  
  delay(500);
}

// void getNumYin(int cnum){
//   int m = cnum * 2;
//   int n = (cnum * 2) - 1;

//   unsigned int gstr1 = numDic[n];
//   unsigned int gstr2 = numDic[m];
// } 应该改用面向对象编程思想的类写法更好，因为需要回传两个返回值



////读一段文字&print函数
void synout(unsigned int yyd[] , int len)
{
  int i = 0;
  unsigned int yuyindata[206]; //文本字符200+6个控制字符
  unsigned int yihuo = 0;

  //yuyin组装前缀控制符
  yuyindata[0] = 0xFD;
  yuyindata[1] = 0x00;
  yuyindata[2] = len + 3;
  yuyindata[3] = 0x01;
  yuyindata[4] = 0x00; //GB2312
  //yuyin组装正文
  for (int i = 0; i < len; i++)
  {
    yuyindata[i + 5] = yyd[i];
  }
  //循环输出 按全长度
  for (int i = 0; i < len + 5; i++)
  {
    yihuo = yihuo ^ yuyindata[i];
    // Serial.println();
    Serial.write(yuyindata[i]);
    softSerial1.write(yuyindata[i]);
  }
  //输出校验码
    Serial.write(yihuo);
    softSerial1.write(yihuo);
  delay(len*200);//按照手册间隔1ms
}





//:天气播报
void speechWea(int wCode){
  if (wCode <=4 && wCode >= 0)
  {
    Serial.println("天气晴朗");

    /* code */
  }else if (wCode = 9)
  {
    Serial.println("阴天");
    int length11 = sizeof(cloudyDic) / sizeof(unsigned int); //度
      synout(cloudyDic, length11);
    /* code */
  }
  
  
}






//:温度播报

unsigned int weiNum = 0x00;
unsigned int str1 = 0x00;
unsigned int str2 = 0x00;


void speechTemp(int temp1)
{
  unsigned int i = 0;
  if (temp1 <= 0){
    if (temp1 == 0){
        weiNum = 0x05;
        int geWei = temp1;
        unsigned int str4[] = {numZero[0], numZero[1]};
        int length1 = sizeof(textN2) / sizeof(unsigned int); //今天江门温度是
        synout(textN2, length1);
        length1 = 0;
        int length2 = sizeof(str4) / sizeof(unsigned int);//温度值 0
        synout(str4, length2);
        delay(200);
        length1 = sizeof(textN1) / sizeof(unsigned int); //度
        synout(textN1, length1);
      }
    else {
      weiNum = 0x07;
    }
  }else { 
    if (temp1 <= 10)
    {
      weiNum = 0x05;
      Serial.println("当前温度是");
      Serial.println(temp1);
      int geWei = temp1;
      unsigned int str4[] = {numDic[(2*geWei)-2], numDic[(2*geWei)-1]};

      int length1 = sizeof(textN2) / sizeof(unsigned int); //今天江门温度是
      synout(textN2, length1);
      length1 = 0;

      int length2 = sizeof(str4) / sizeof(unsigned int);//温度值
      synout(str4, length2);


      delay(200);
      length1 = sizeof(textN1) / sizeof(unsigned int); //度
      synout(textN1, length1);


    }
    else if (temp1 > 10 && temp1 < 20)
    {
      weiNum = 0x07;
      //int shiWei = temp1 / 10;
      unsigned int str3[] = {numDic[18], numDic[19]};
      int geWei = temp1 - 10;
      unsigned int str4[] = {numDic[(2*geWei)-2], numDic[(2*geWei)-1]};
      
      length1 = sizeof(textN2) / sizeof(unsigned int); //今天江门温度是
      synout(textN2, length1);
      length1 = 0;



      int length2 = sizeof(str3) / sizeof(unsigned int); //温度值
      synout(str3, length2);
      delay(200);
      length2 = sizeof(str4) / sizeof(unsigned int);//温度值
      synout(str4, length2);
      delay(200);
      length1 = sizeof(textN1) / sizeof(unsigned int); //度
      synout(textN1, length1);


    }
    else if (temp1 == 20)
    {
      int shiWei = temp1 / 10;
      unsigned int str3[] = {numDic[(shiWei * 2) - 2], numDic[(shiWei * 2) - 1]};
      unsigned int str5[] = {numDic[18], numDic[19]}; //十
      length1 = sizeof(textN2) / sizeof(unsigned int); //今天江门温度是
      synout(textN2, length1);
      length1 = 0;

      int length2 = sizeof(str3) / sizeof(unsigned int); //温度值
      synout(str3, length2);
      delay(200);
      length2 = sizeof(str5) / sizeof(unsigned int); //温度值
      synout(str5, length2);
      delay(200);
      length1 = sizeof(textN1) / sizeof(unsigned int); //度
      synout(textN1, length1);
      
    }
    else if (temp1 > 20)
      { 
        if((temp1%10)==0){
          int shiWei = temp1 / 10;
          unsigned int str3[] = {numDic[(shiWei*2)-2], numDic[(shiWei*2)-1]};
          unsigned int str5[] = {numDic[18], numDic[19]}; //十

          length1 = sizeof(textN2) / sizeof(unsigned int); //今天江门温度是
          synout(textN2, length1);
          length1 = 0;

          int length2 = sizeof(str3) / sizeof(unsigned int); //温度值
          synout(str3, length2);
          delay(200);
          length2 = sizeof(str5) / sizeof(unsigned int); //温度值
          synout(str5, length2);
          delay(200);
          length1 = sizeof(textN1) / sizeof(unsigned int); //度
          synout(textN1, length1);
        }else{//不是30、40这种整十时候
        weiNum = 0x09;
        Serial.println("三位啊"); // 例如 21 语音就会读 二 十 一 三个字所以就 三位啊        
        int shiWei = temp1 / 10;
        int geWei = temp1 - (shiWei * 10);
        unsigned int str3[] = {numDic[(shiWei*2)-2], numDic[(shiWei*2)-1]};
        unsigned int str5[] = {numDic[18], numDic[19]}; //十
        unsigned int str4[] = {numDic[(2*geWei)-2], numDic[(2*geWei)-1]};
        length1 = sizeof(textN2) / sizeof(unsigned int); //今天江门温度是
        synout(textN2, length1);
        length1 = 0;



        int length2 = sizeof(str3) / sizeof(unsigned int); //温度值
        synout(str3, length2);
        delay(200);
        length2 = sizeof(str5) / sizeof(unsigned int); //温度值
        synout(str5, length2);
        delay(200);
        length2 = sizeof(str4) / sizeof(unsigned int);//温度值
        synout(str4, length2);
        delay(200);
        length1 = sizeof(textN1) / sizeof(unsigned int); //度
        synout(textN1, length1);
        }
      }
  }
}



void speechJM2(){
  unsigned int i = 0;
  //unsigned int j = 0;

  // for (j = 0; j < sizeof(text3); j++)
  // {
  //   softSerial1.write(text3[j]);
  // }
  for (i = 0; i < sizeof(text2); i++)
  {
    softSerial1.write(text2[i]);
  }
}
void speechJM(){
  unsigned int i = 0;
  unsigned int j = 0;

  for (j = 0; j < (sizeof(text3)/4); j++)
  {
    softSerial1.write(text3[j]);
  }
  for (i = 0; i < (sizeof(text1)/4); i++)
  {
    softSerial1.write(text1[i]);
  }
}

void speech(){
  unsigned char i = 0;
  unsigned char head[20];

  head[0] = 0xFD;
  head[1] = 0x00;
  head[2] = 0x11;
  head[3] = 0x01;
  head[4] = 0x00;
  head[5] = 0xCF;
  head[6] = 0xD6;
  head[7] = 0xD4;
  head[8] = 0xDA;
  head[9] = 0xBD;
  head[10] = 0xAD;
  head[11] = 0xC3;
  head[12] = 0xC5;
  head[13] = 0xCE;
  head[14] = 0xC2;
  head[15] = 0xB6;
  head[16] = 0xC8;
  head[17] = 0xCA;
  head[18] = 0xC7;
  head[19] = 0x93;

  for(i=0; i<20; i++){
    softSerial1.write(head[i]);
    //Serial.write(head[i]);
  }
}

class Syn6288 {
  public:
  uint8_t music;
  uint8_t TEXTLEN;
  uint8_t pi;
  void play(uint8_t *text,uint8_t TEXTLEN,uint8_t music);
//void play(uint8_t *text,uint8_t music);
  void Slaveboudset(uint16_t boudr);
  void stop();
  void restore();
  void inquire();
  void Pause();
  void sleep();
};



void get_DHT(){
  float h = dht.readHumidity(); //获取湿度
  float t = dht.readTemperature();//获取温度
  dhtH = h;
  dhtT = t;
  Serial.println(h);
  Serial.println(t);
  delay(1000);
  Serial.println("传感器温度获取完成");
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
        String nowWeaCodeYY = results_0_now["code"]; // "0"
        const char* results_0_now_temperature = results_0_now["temperature"]; // "5"
        String nowTempYY = results_0_now["temperature"];
        const char* results_0_last_update = results_0["last_update"]; // "2020-11-23T15:20:00+08:00"

        nnowWeather = results_0_now_text;
        int nnowWeatherCode = nowWeaCodeYY.toInt();
        nWCode = nnowWeatherCode;//赋给全局变量天气代码
        nowTemp = results_0_now_temperature;
        nowTempInt = nowTempYY.toInt();
          Serial.println(nowTempInt);
        http.end();
        Serial.println("江门天气是:");
        Serial.println(nnowWeather);
        Serial.println("温度:");
        Serial.println(nowTemp);
        Serial.println("当前天气code");
        Serial.println(nnowWeatherCode);
        get_DHT(); //调用传感器函数

        //u8g2.setFont(u8g2_font_unifont_t_chinese2);

        // u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
        // u8g2.setFontDirection(0);
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
            u8g2.setCursor(0,40);
            u8g2.print("室内温度:");
            u8g2.setCursor(50,40);
            u8g2.print(dhtT);
            u8g2.setCursor(0,55);
            u8g2.print("室内湿度:");
            u8g2.setCursor(50,55);
            u8g2.print(dhtH);

          } while ( u8g2.nextPage() );
          delay(1000);
          // speechTemp(nowTempInt);
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
