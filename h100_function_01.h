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

#ifndef h100_function_01_h
#define h100_function_01_h
int function_01_number;
// ws2812fx.setMode(30); 这个程序灯带会闪

void function_01(int function_01_number)
{
  switch(function_01_number)
  {
    case 0:
      ws2812fx.setColor(0xFF0000);
      ws2812fx.setMode(30);
      //ws2812fx.start();  
      Serial.printf("function_01_number in case 0 is 0x%02X\n", function_01_number);
      delay(2000);
      break;
    case 1:
      ws2812fx.setColor(0x00FF00);
      ws2812fx.setMode(15);
      //ws2812fx.start();
      Serial.printf("function_01_number in case 1 is 0x%02X\n", function_01_number);
      delay(2000);
      break;
  
    case 2:
      ws2812fx.setColor(0x0000FF);
      ws2812fx.setMode(16);
      //ws2812fx.start();
      Serial.printf("function_01_number in case 2 is 0x%02X\n", function_01_number);
      delay(2000);
      break;
    
    case 3:
      ws2812fx.setColor(0x00FFFF);
      ws2812fx.setMode(18);
      //ws2812fx.start();
      Serial.printf("function_01_number in case 3 is 0x%02X\n", function_01_number);
      delay(2000);
      break;

    default:
      Serial.printf("function_01_number in default is 0x%02X\n", function_01_number);
      
      delay(3000);
      break;
  }
  

}


#endif