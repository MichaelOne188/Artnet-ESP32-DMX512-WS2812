#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef WiFi_h
#include <WiFi.h>
#endif

#ifndef EEPROM_h
#include <EEPROM.h> //导入Flash库文件
#endif

//#include "h100_adafruit.h"

#ifndef h100_ws2812fx_h
#define h100_ws2812fx_h

//#define LED_COUNT 10
#define LED_PIN 22  

//下面是关于灯带的一些设置
#define REDUCED_MODES    // sketch too big for Arduino Leonardo flash, so invoke reduced modes

#define STRIP_BRIGHT    led_address+0
#define STRIP_WHITE     led_address+1     //注意现在白光和红光是同一个值
#define STRIP_RED       led_address+1
#define STRIP_GREEN     led_address+2
#define STRIP_BLUE      led_address+3
#define STRIP_MODE      led_address+4
#define STRIP_SPEED     led_address+5
#define STRIP_REVERSE   led_address+6 

extern EEPROMClass  EEPROM_WiFi;
extern EEPROMClass  EEPROM_dmxData;
extern EEPROMClass  EEPROM_led_address;
extern int LED_COUNT;
//设置控制器作为灯带解码器的初始地址
extern short led_address;

WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


//led setting
void set_ws2812fx_led(uint8_t *led_data)
{
  Serial.println("led_address");
  Serial.println(led_address);
  Serial.println(STRIP_BRIGHT);
  ws2812fx.setBrightness(led_data[STRIP_BRIGHT]);
// ws2812fx.setSegment(0,  0起点,  51终点, FX_MODE_COLOR_WIPE, 0xFF0000, 1000, false); // segment 0 is leds 0 - 9
  uint8_t   w=led_data[STRIP_WHITE];
  uint8_t   r=led_data[STRIP_RED];
  uint8_t   g=led_data[STRIP_GREEN];
  uint8_t   b=led_data[STRIP_BLUE];
  //uint8_t   led_color[]={r,g,b};
  uint32_t  led_color=((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
  uint8_t   led_mode=led_data[STRIP_MODE]/4;
  uint16_t  led_speed=256*led_data[STRIP_SPEED];
  bool      led_reverse=(led_data[STRIP_REVERSE]>127)?true:false;

  ws2812fx.setSegment(0,  0,  LED_COUNT, led_mode, led_color, led_speed, led_reverse); // segment 0 is leds 0 - 9
  Serial.println("***************led color************");
  Serial.println(led_color, HEX); 
  Serial.println("***************led mode************");
  Serial.println(led_mode); 
  
}

void set_ws2812fx_rgbstatic(uint8_t *led_data)
{
  uint8_t   w=led_data[led_address+0];
  uint8_t   r=led_data[led_address+0];
  uint8_t   g=led_data[led_address+1];
  uint8_t   b=led_data[led_address+2];
  uint32_t led_color=((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
  ws2812fx.setBrightness(255);
  ws2812fx.setColor(led_color);
  ws2812fx.setMode(FX_MODE_STATIC);
 
}

void dmx_ws2812fx(uint8_t *dmx_led_data)
{
  //ws2812fx.setBrightness(dmx_led_data[led_address+0]);
  ws2812fx.setBrightness(255);
  uint8_t   w=dmx_led_data[led_address+1];
  uint8_t   r=dmx_led_data[led_address+1];
  uint8_t   g=dmx_led_data[led_address+2];
  uint8_t   b=dmx_led_data[led_address+3];
  uint32_t led_color=((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
  ws2812fx.setColor(led_color);

  if(dmx_led_data[led_address+4]>=72)
    {
      ws2812fx.setMode(72);
    }
  else
    {
      ws2812fx.setMode(dmx_led_data[led_address+4]);
    }
  
  ws2812fx.setSpeed(dmx_led_data[led_address+5]*10);
  //delay(300);

  //Serial.printf("Sending led_address+4  is  0x%02X\n", dmx_led_data[led_address+4]);
 
}
#endif