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

#ifndef h100_function_00_h
#define h100_function_00_h
int function_00_number;

void function_00(int function_00_number)
{
  switch(function_00_number)
  {
    case 0:
    for(int j=1;j<=510;j++)
      {
        for (int i = 1; i < 512; i++) 
        {
          if(function_00_number!=0){break;}
          if(j<=255){dmxData[i] = j;}
          else{dmxData[i]=510-j;}  
        }
        dmx_output(0,0);
      }
      break;

    case 1:
    for(int j=3;j<=510;j=j+3)
      {
        for (int i = 1; i < 512; i++) 
          {
            if(function_00_number!=1){break;}
            if(j<=255){dmxData[i] = j;}
            else{dmxData[i]=510-j;}  
          }
      dmx_output(0,0);
      }
    break;
  
  case 2:
    for(int n=1;n<512;n++)
      {
        dmxData[n]=50;
      }
      dmx_output(0,0);
      delay(500);
    for(int m=1;m<512;m++)
      {
        dmxData[m]=0;
      }
      dmx_output(0,0);
      delay(500);
      break;

  default:
    
    delay(3000);
    break;
  }
  Serial.printf("function_00_number is 0x%02X\n", function_00_number);
}


#endif