#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef WiFi_h
#include <WiFi.h>
#endif

#ifndef EEPROM_h
#include <EEPROM.h> //导入Flash库文件
#endif

#ifndef h100_address_h
#define h100_address_h

extern int LED_COUNT;

//设置控制器作为灯带解码器的初始地址
short led_address=1;

//下面是led_address的保存和取用函数

void save_led_address()//保存函数
{ 
  if(led_address>500){led_address=500;} 
  if(led_address<=0){led_address=1;} 
  EEPROM_led_address.begin(2);//向系统申请2byte ROM
  //开始写入
  EEPROM_led_address.write(100, led_address); //在闪存内模拟写入
  EEPROM_led_address.commit();//执行写入ROM
}

void load_led_address()//读取函数
{ 

  EEPROM_led_address.begin(2);
  led_address= EEPROM_led_address.read(100);
  EEPROM_led_address.commit();

}

#endif