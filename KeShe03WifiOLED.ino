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

#define DHTPIN 2 //D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE); //实例化DHT C++风格
SoftwareSerial softSerial1(14,12); //实例化软串口 rx使用D5 io14 与syn6288 tx相连 、 tx使用D6 io12引脚与syn6288 rx相连
//以下是现在江门温度是和数字的语音字节码
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
//以下是天气语音字节码
unsigned int overcastDic[] = {0xD2, 0xf5, 0xCC, 0xEc};//阴天
unsigned int sunnyDic[] = {0xC7, 0xE7, 0xCC, 0xEC};//晴天
unsigned int cloudyDic[] = {0xB6, 0xE0, 0xD4, 0xC6};//多云
unsigned int partlycloudyDic[] = {0xC7,0xE7,0xBC,0xE4,0xB6,0xE0,0xD4,0xC6};//晴间多云
unsigned int mostlycloudyDic[] = {0xB4,0xF3,0xB2,0xBF,0xB6,0xE0,0xD4,0xC6};//大部多云
unsigned int showerDic[] = {0xD5, 0xF3, 0xD3, 0xEA}; //阵雨
unsigned int thundershowerDic[] = {0xC0, 0xD7, 0xD5, 0xF3, 0xD3, 0xEA};//雷阵雨
unsigned int xiaorainDic[] = { 0xD0,0xA1,0xD3,0xEA};//小雨
unsigned int zhongrainDic[] = { 0xD6,0xD0,0xD3,0xEA};//
unsigned int darainDic[] = { 0xB4,0xF3,0xD3,0xEA};//da雨
unsigned int stormDic[] = { 0xB1,0xA9,0xD3,0xEA};//bao雨
unsigned int hazeDic[] = { 0xD3,0xD0,0xF6,0xB2};//mai
unsigned int foggyDic[] = { 0xD3,0xD0,0xCE,0xED};//wu
unsigned int windyDic[] = { 0xD3,0xD0,0xB7,0xE7};//有风
unsigned int dawindyDic[] = { 0xB4,0xF3,0xB7,0xE7};//大风
unsigned int coldDic[] = { 0xC0,0xE4,0xCC,0xEC};//冷


U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE);   // 参照ESP32 Thing, pure SW emulated I2C esp8266引脚 D1给SDL D2给CLK

HTTPClient http; //实例化一个httpclinet请求类 C++ 写法 用于后边http网络请求

//以下是初始化温度变量和天气信息变量以及传感器温度湿度变量以及
//语音播报状中断控制状态码、OLED显示器中断控制状态码、天气状态码
String nnowWeather = "";
String nowTemp = "";
int test21 = 21;
float dhtH = 1.1;
float dhtT = 1.1;
int nowTempInt = 23;
int nWCode = 0; //天气状态码
int length1 = 0; 
int zongduanState = 0;//语音播报状态码
int oledState = 1; //OLED状态码1显示温度 2显示对比 3北上广

//北上广天气和温度储存变量初始化
String beijingWea = "";
String beijingTemp = "";
String shanghaiWea = "";
String shanghaiTemp = "";
String guangzhouWea = "";
String guangzhouTemp = "";

//创建一个叫温度对比（网络与传感器对比）的C++类
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


//setup()是单片机主程序入口，相当于标准C程序的main函数，但setup函数是初始化只允许一次
void setup(){
  // put your setup code here, to run once:
  
  Serial.begin(115200);
  pinMode(13, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
  attachInterrupt(13, lowInterrupt, FALLING); //按键中断用D7对应13 引脚
  attachInterrupt(0, D3Interrupt, FALLING);//按键中断D3对应0引脚
  softSerial1.begin(9600);
  softSerial1.listen();
  dht.begin();

  Serial.println();
  u8g2.begin();   //选择U8G2模式，或者U8X8模式
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312a);
  u8g2.setFontDirection(0);
  get_WIFI();//调用WiFi函数以启动WiFi并连接
  attachInterrupt(13, lowInterrupt, FALLING);
  delay(5000);
  httpWeatherBSG();//北上广天气请求
  httpWeather();//江门天气请求
  //httpWeather();
  
}
//设置D7引脚按键的中断运行的函数 
//更改zongduanState语音播报状态码以触发语音播报
ICACHE_RAM_ATTR void lowInterrupt(){
  //led = -led;
  //Serial.println("66666666");
  //speechTemp(22);
  zongduanState = 1;
  detachInterrupt(13);
  attachInterrupt(13, lowInterrupt, FALLING);
  detachInterrupt(0);
  attachInterrupt(0, D3Interrupt, FALLING);
}
//设置D3引脚按键的中断运行的函数 
//更改oled状态码以触发OLED显示器画面改变
ICACHE_RAM_ATTR void D3Interrupt(){
  //led = -led;
  //Serial.println("D3D3D3");
  //speechTemp(22);
  //oledState = -(oledState);
  Serial.println("当前显示状态");
  Serial.println(oledState);

  switch (oledState)
  {
  case 1:
    oledState = 2;
    break;
  case 2:
    oledState = 3;
    break;
  case 3:
    oledState = 1;
    break;

  default:
    oledState = 1;
    break;
  }
  Serial.println("按下变更状态");
  Serial.println(oledState);

  detachInterrupt(13);
  attachInterrupt(13, lowInterrupt, FALLING);
  detachInterrupt(0);
  attachInterrupt(0, D3Interrupt, FALLING);
  
  
}

//loop()函数是单片机运行setup（）函数之后会循环运行的函数，
//可以看做是main函数的一部分
void loop() {
  // put your main code here, to run repeatedly:
  //speechJM();
  get_DHT();//调用传感器函数
  //switch语句判断OLED显示状态码以正确显示当前OLED内容
  switch (oledState)
  {
  case 1: //页面1显示江门天气和传感器信息
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
    break;
  case 2://页面2显示温度对比

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
      break;
    default:
      u8g2.print("没比较error");
      // u8g2.print(wddb.chaZhi);
      // u8g2.print("摄氏度");
      break;
    }

    } while ( u8g2.nextPage() );
    break;
  case 3://页面3显示背上广
    u8g2.firstPage();
    do{
    u8g2.setCursor(10, 10);
    u8g2.print("江门天气:");
    u8g2.print(nnowWeather);
    u8g2.setCursor(0, 25);
    u8g2.print("北京天气:");
    u8g2.print(beijingWea);
    u8g2.print("温度:");
    u8g2.print(beijingTemp);
    u8g2.setCursor(0,40);
    u8g2.print("上海天气:");
    u8g2.print(shanghaiWea);
    u8g2.print("温度:");
    u8g2.print(shanghaiTemp); 
    u8g2.setCursor(0,55);
    u8g2.print("广州天气:");
    u8g2.print(guangzhouWea);
    u8g2.print("温度:");
    u8g2.print(guangzhouTemp);
    } while ( u8g2.nextPage() );
    break;

  default:
    break;
  }
  


  String testt4 = "20";
  String qt = testt4;

//当语音播报状态码为1则播报语音
  if (zongduanState == 1)
  {
    zongduanState = 0;
    Serial.println(zongduanState);
    //speechTemp(10);
    speechTemp(nowTempInt);//播报江门温度
    delay(200);
    speechWea(nWCode);//播报天气
    delay(500);
  }



  
  delay(500);
}

// void getNumYin(int cnum){
//   int m = cnum * 2;
//   int n = (cnum * 2) - 1;

//   unsigned int gstr1 = numDic[n];
//   unsigned int gstr2 = numDic[m];
// } 原猜想应该改用面向对象编程思想的类写法更好，因为需要回传两个返回值
//但是没用上



////读一段文字&print函数 输入要读的文本字节和长度
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
  if (wCode <=3 && wCode >= 0)
  {
    Serial.println("天气晴朗");
    Serial.println("阴天");
    int length11 = sizeof(sunnyDic) / sizeof(unsigned int); //晴天
      synout(sunnyDic, length11);
    /* code */
  }else if (wCode >=4)
  {
    int length11;
    switch (wCode)
    {
    case 4:
      length11 = sizeof(cloudyDic) / sizeof(unsigned int); //多云
      synout(cloudyDic, length11);
      break;
    case 5:
      length11 = sizeof(partlycloudyDic) / sizeof(unsigned int); //局部多云
      synout(partlycloudyDic, length11);
      break;
    case 6:
      length11 = sizeof(partlycloudyDic) / sizeof(unsigned int); //局部多云
      synout(partlycloudyDic, length11);
      break;
    case 7:
      length11 = sizeof(mostlycloudyDic) / sizeof(unsigned int); //大部多云
      synout(mostlycloudyDic, length11);
      break;
    case 8:
      length11 = sizeof(mostlycloudyDic) / sizeof(unsigned int); //大部多云
      synout(mostlycloudyDic, length11);
      break;
    case 9:
      Serial.println("阴天");
      length11 = sizeof(overcastDic) / sizeof(unsigned int); //阴
      synout(overcastDic, length11);
      break;
    case 10:
      length11 = sizeof(showerDic) / sizeof(unsigned int); //阵雨
      synout(showerDic, length11);
      break;
    case 13:
      length11 = sizeof(xiaorainDic) / sizeof(unsigned int); //小雨
      synout(xiaorainDic, length11);
      break;

    case 14:
      length11 = sizeof(zhongrainDic) / sizeof(unsigned int); //中雨
      synout(zhongrainDic, length11);
      break;
    case 15:
      length11 = sizeof(darainDic) / sizeof(unsigned int); //大雨
      synout(darainDic, length11);
      break;
    case 16:
      length11 = sizeof(stormDic) / sizeof(unsigned int); //暴雨
      synout(stormDic, length11);
      break;
    case 30:
      length11 = sizeof(foggyDic) / sizeof(unsigned int); //雾
      synout(foggyDic, length11);
      break;
    case 31:
      length11 = sizeof(hazeDic) / sizeof(unsigned int); //雾霾
      synout(hazeDic, length11);
      break;
    case 32:
      length11 = sizeof(windyDic) / sizeof(unsigned int); //风
      synout(windyDic, length11);
      break;
    case 33:
      length11 = sizeof(dawindyDic) / sizeof(unsigned int); //大风
      synout(dawindyDic, length11);
      break;
    case 37:
      length11 = sizeof(coldDic) / sizeof(unsigned int); //冷
      synout(coldDic, length11);
      break;
    default:
      
      length11 = sizeof(sunnyDic) / sizeof(unsigned int); //
      synout(sunnyDic, length11);
      
      break;
    }
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


//温度湿度传感器函数
void get_DHT(){
  float h = dht.readHumidity(); //获取湿度
  float t = dht.readTemperature();//获取温度
  dhtH = h;
  dhtT = t;
  Serial.println(h);
  Serial.println(t);
  delay(1000);
  Serial.println("传感器湿度温度获取完成");
}
//WiFi连接函数
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
//江门天气请求与获取于JSON数据处理函数
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

//背上广天气请求与获取于JSON数据处理函数
void httpWeatherBSG(){
  if (WiFi.status() == WL_CONNECTED){
    http.begin("http://api.seniverse.com/v3/weather/now.json?key=Sehu5Cll8Qd7jX5_Z&location=beijing&language=zh-Hans&unit=c");//beijing weather
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
        //String nowWeaCodeYY = results_0_now["code"]; // "0"
        const char* results_0_now_temperature = results_0_now["temperature"]; // "5"
        //String nowTempYY = results_0_now["temperature"];
        const char* results_0_last_update = results_0["last_update"]; // "2020-11-23T15:20:00+08:00"
        beijingWea = results_0_now_text;
        beijingTemp = results_0_now_temperature;
        Serial.println("北京天气是:");
        Serial.println(beijingWea);
        Serial.println("温度:");
        Serial.println(beijingTemp);
        delay(100);
      }else {
        Serial.printf(http.errorToString(httpCode).c_str());

      }

      http.end();

    }else{
      Serial.println("unable to connect");
    }
  }
  delay(10);
  if (WiFi.status() == WL_CONNECTED){
    http.begin("http://api.seniverse.com/v3/weather/now.json?key=Sehu5Cll8Qd7jX5_Z&location=shanghai&language=zh-Hans&unit=c");//shanghai weather
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
        //String nowWeaCodeYY = results_0_now["code"]; // "0"
        const char* results_0_now_temperature = results_0_now["temperature"]; // "5"
        //String nowTempYY = results_0_now["temperature"];
        const char* results_0_last_update = results_0["last_update"]; // "2020-11-23T15:20:00+08:00"
        shanghaiWea = results_0_now_text;
        shanghaiTemp = results_0_now_temperature;
        Serial.println("上海天气是:");
        Serial.println(shanghaiWea);
        Serial.println("温度:");
        Serial.println(shanghaiTemp);
        delay(100);
      }else {
        Serial.printf(http.errorToString(httpCode).c_str());

      }

      http.end();

    }else{
      Serial.println("unable to connect");
    }
  }
  delay(10);
  if (WiFi.status() == WL_CONNECTED){
    http.begin("http://api.seniverse.com/v3/weather/now.json?key=Sehu5Cll8Qd7jX5_Z&location=guangzhou&language=zh-Hans&unit=c");//beijing weather
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
        //String nowWeaCodeYY = results_0_now["code"]; // "0"
        const char* results_0_now_temperature = results_0_now["temperature"]; // "5"
        //String nowTempYY = results_0_now["temperature"];
        const char* results_0_last_update = results_0["last_update"]; // "2020-11-23T15:20:00+08:00"
        guangzhouWea = results_0_now_text;
        guangzhouTemp = results_0_now_temperature;
        Serial.println("广州天气是:");
        Serial.println(guangzhouWea);
        Serial.println("温度:");
        Serial.println(guangzhouTemp);
        delay(100);
      }else {
        Serial.printf(http.errorToString(httpCode).c_str());

      }

      http.end();

    }else{
      Serial.println("unable to connect");
    }
  }
}