#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef WiFi_h
#include <WiFi.h>
#endif

#ifndef EEPROM_h
#include <EEPROM.h> //导入Flash库文件
#endif

#ifndef Adafruit_NeoPixel_h
#include <Adafruit_NeoPixel.h>
#endif

#ifndef h100_ws2812fx_h
#include "h100_ws2812fx.h"
#endif

#ifndef h100_adafruit_h
#include "h100_adafruit.h"
#endif

#ifndef h100_address_h
#include "h100_address.h"
#endif

#ifndef h100_function_07_h
#define h100_function_07_h

extern EEPROMClass  EEPROM_speed_ws2811;
int function_07_number;
extern int speed_ws2811;

void save_speed_ws2811()//保存函数
{ 
 
  EEPROM_speed_ws2811.begin(100);//向系统申请100byte ROM
  //开始写入
  EEPROM_speed_ws2811.write(2, speed_ws2811); //在闪存内模拟写入
  EEPROM_speed_ws2811.commit();//执行写入ROM

}

void load_speed_ws2811()//读取函数
{ 

  EEPROM_speed_ws2811.begin(100);
  speed_ws2811= EEPROM_speed_ws2811.read(2);

}





void function_07_1()
{
  for (int i = 0;i<=LED_COUNT; i++)
   {
      strip.setPixelColor(i, 255,0,0);
      strip.show();
      delay(5*speed_ws2811);
      strip.setPixelColor(i, 255,255,0);
      strip.show();
      delay(5*speed_ws2811);
      strip.setPixelColor(i, 255,255,255);
      strip.show();
      delay(5*speed_ws2811);
      }           
}




void function_07_speedadjust(int basic_speed,int add_number)
{
  if(speed_ws2811<100)
  {
    speed_ws2811=basic_speed+add_number;
  }
  else
  {
    //这个值要注意，可能会影响灯带速度大局；
    speed_ws2811=0;
  }
  save_speed_ws2811(); 
  delay(200);    
}


#endif