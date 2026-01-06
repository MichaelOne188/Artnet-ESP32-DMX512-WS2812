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

#ifndef h100_dmx_h
#define h100_dmx_h

extern EEPROMClass  EEPROM_WiFi;
extern EEPROMClass  EEPROM_dmxData;
extern EEPROMClass  EEPROM_led_address;




const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

static uint32_t lastdmxdatasavetime = 0;

int16_t led_universe=0;
uint32_t timeinterval_save_dmxdata=300000;
bool savedmx_or_not;
extern byte dmxData[DMX_PACKET_SIZE]; //=513
extern dmx_port_t dmx_Port ;
extern short led_address;


// dmx output while no artnet data received
void dmx_output(uint32_t timeinterval_save_dmxdata,bool savedmx_or_not)
{
  dmx_write(dmx_Port, dmxData, DMX_PACKET_SIZE);
  dmx_send(dmx_Port, DMX_PACKET_SIZE);
    /* If we have no more work to do, we will wait until we are done sending our
    DMX packet. */
    //下面这个函数要再了解
  dmx_wait_sent(dmx_Port, DMX_TIMEOUT_TICK);

  //每5分钟保存一次DMX512通道值5*60*1000=300000
  if((millis()-lastdmxdatasavetime>timeinterval_save_dmxdata)&&savedmx_or_not)
  {
    EEPROM_dmxData.begin(DMX_PACKET_SIZE);
    for (int i = 1; i < DMX_PACKET_SIZE; i++) 
    {
      EEPROM_dmxData.write(i, dmxData[i]);
    }
    EEPROM_dmxData.commit();
    delay(100);
    lastdmxdatasavetime = millis();
  }
}

//保存DMX
void dmx_save(uint32_t timeinterval_save_dmxdata)
{
  
  //每5分钟保存一次DMX512通道值5*60*1000=300000
  if(millis()-lastdmxdatasavetime>timeinterval_save_dmxdata)
  {
    EEPROM_dmxData.begin(DMX_PACKET_SIZE);
    for (int i = 1; i < DMX_PACKET_SIZE; i++) 
    {
      EEPROM_dmxData.write(i, dmxData[i]);
    }
    EEPROM_dmxData.commit();
    delay(100);
    lastdmxdatasavetime = millis();
  }
}

//  回调函数 ,注意data是从0索引开始
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  led_universe=universe;
  //这个赋值语句有时间需优化，for语句耗时太长。可以的话改用指针。
  //Serial.printf("Universe is %u\n",universe);
  //Serial.printf("Receiving universe  0x%02X\n", universe);
  //Serial.printf("Sending DMX5 is  0x%02X\n", dmxData[5]);
  for (int i = 1; i < DMX_PACKET_SIZE; i++)
    {
      dmxData[i]=data[i-1];
    }
  Serial.printf("Sending DMX5 is  0x%02X\n", dmxData[5]);
  //第0值为0，
  //dmx_output(300000,1);
  /*if(universe==0||universe==1)
    {
      for (int i = 1; i < DMX_PACKET_SIZE; i++)
        {dmxData[i]=data[i-1];}//第0值为0，
         dmx_output(300000,1);
    }
   // set_ws2812fx_rgbstatic(dmxData);
    //set_adafruit_rgbstatic(dmxData);
    //dmx_output();

    /*Serial.printf("the decoder address is =============================================================:");
    Serial.println(led_address);
    Serial.printf("Sending DMX value of led_address is  0x%02X\n", dmxData[led_address]);
    Serial.printf("Universe is %u\n",universe);
  
  else if(universe==2||universe==3){
    Serial.printf("Universe is %u\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",universe);
    set_adafruit_artnettopixel(dmxData,universe);
    }
  
  // 暂时设定空间17第1，2通道值的和为led端口的DMX512地址
  else if (universe==17) {
    led_address=data[0]+data[1];
    save_led_address();
    Serial.printf("you are setting dmx address by Universe  %u\n channel 1 and channel 2",universe);
    Serial.printf("the decoder address is =============================================================:");
    Serial.println(led_address);
    }
    
  else {
    Serial.printf("Universe is %u\n",universe);
    Serial.printf("Sending DMX5 is  0x%02X\n", dmxData[5]);
    }
    */
  
}



#endif