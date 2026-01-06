
#include <Arduino.h>
#include <ArtnetWifi.h>
#include <esp_dmx.h>
#include <WiFi.h>
#include <EEPROM.h> //导入Flash库文件
#include <WS2812FX.h>
#include <Adafruit_NeoPixel.h>
#include "h100_wifi.h"
#include "h100_dmx.h"
#include "h100_ws2812fx.h"
#include "h100_adafruit.h"
#include "h100_address.h"
#include "h100_function_00.h"
#include "h100_function_01.h"
#include "h100_function_07.h"

//使用注意事项：在STA和AP模式下，在不连接控台情况下，均可以更改DMX512地址，更改完成后停留3秒退出软件，便可以重新连接控台。
//在STA断网后，会切换到AP模式，在这个阶段，解码器不能用，等AP模式成功后，可以正常解码
int LED_COUNT=60;
int speed_ws2811=10;

EEPROMClass  EEPROM_WiFi("eeprom0");
EEPROMClass  EEPROM_dmxData("eeprom1");
EEPROMClass  EEPROM_led_address("eeprom2");
EEPROMClass  EEPROM_speed_ws2811("eeprom3");


extern int16_t led_universe;
extern short led_address;

// Art-Net settings
ArtnetWifi artnet;

byte dmxData[DMX_PACKET_SIZE];
byte dmxonlineData[DMX_PACKET_SIZE];
unsigned long lastUpdate = millis();
bool dmxIsConnected = false;

//定义控制器引脚，其中灯带输出引脚放在h100_led的宏定义中
const int dmx_transmitPin = 17;
const int dmx_receivePin = 16;
const int dmx_enablePin = 15;
const int flagPin = 5;   //LED灯指示引脚
//定义拔码开关引脚
const int pin_sw1=27;
const int pin_sw2=26;
const int pin_sw3=25;
const int pin_sw4=14;
const int pin_sw5=12;
//定义前4位拔码开关引脚的状态变量
int switchValue = 0;
//定义第5位拔码开关引脚的状态变量
bool wifimodeValue =0;


//配网变量
bool flag_smartconfigwifi=false;  //    false: 不一键配网  ture:  开始一键配网



//这个可能只是表示DMX用的是哪一个串口。因为0用来和电脑通讯，所以用1。
dmx_port_t dmx_Port = 1;

//下面是关于引脚为0按键的一些设置

struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};
//button0是按键，button21是传感器

Button button0 = {0, 0, false};
Button button4 = {4, 0, false};
// Variables will change:
int button4PushCounter = 0;  // counter for the number of button presses
int button4State = 0;        // current state of the button
int lastButton4State = 0;    // previous state of the button


//这个是按键0的中断程序
void ARDUINO_ISR_ATTR isr() {
    
    button0.numberKeyPresses += 1;
   // flag_smartconfigwifi=true;
    button0.pressed = true;

}

//这个是引脚4的中断程序
void ARDUINO_ISR_ATTR isr_4() {
    
    button4.numberKeyPresses += 1;
   // flag_smartconfigwifi=true;
    button4.pressed = true;
}

//ESP32-WROOM-32D 和 ESP32-WROOM-32U 集成了 4 MB 的 SPI flash，连接 ESP32 的管脚 GPIO6，GPIO7，
//GPIO8，GPIO9，GPIO10 和 GPIO11。这六个管脚不建议用于其他功能。
int function_00_mode_number=5;
int function_01_mode_number=5;
int function_02_mode_number=2000;
int function_04_mode_number=100;
int function_05_mode_number=100;
int function_07_mode_number=0;
int temp_01=100;
int temp_02=100;
int temp_03=100;
int temp_04=100;
int temp_05=100;
int temp_07=0;
int temp_08=0;


void setup()
{ 
  
  
  ws2812fx.init();
  ws2812fx.setBrightness(255);
  ws2812fx.setSpeed(1000);
  ws2812fx.setColor(0x007BFF);
  ws2812fx.setMode(2);
  ws2812fx.start();
   //开启串口输出
  Serial.begin(115200);
  delay(3000);
  Serial.println("setup is running*************************");
  //设置信号指示灯
  pinMode(flagPin, OUTPUT);//设定D5号脚为LED
  //设置拔码开关
  pinMode(pin_sw1,INPUT_PULLUP);
  pinMode(pin_sw2,INPUT_PULLUP);
  pinMode(pin_sw3,INPUT_PULLUP);
  pinMode(pin_sw4,INPUT_PULLUP);
  pinMode(pin_sw5,INPUT_PULLUP);



  //把保存的512地值提出
  Serial.printf("led address in setup address is ");
  load_led_address();
  Serial.println(led_address);

  //连接网络或一键配网
  
  
  
  wifimodeValue=digitalRead(pin_sw5);

  if(wifimodeValue==0)
    { 
      smartconfigafterconnectwifi();
    }
  else if(wifimodeValue==1)
    {
      apConnect();
    }

   //设置引脚0的按键
  pinMode(button0.PIN, INPUT_PULLUP);
  attachInterrupt(button0.PIN, isr, FALLING);


   //初始化DMX512输出
  dmx_config_t config = DMX_CONFIG_DEFAULT;
  dmx_driver_install(dmx_Port, &config, DMX_INTR_FLAGS_DEFAULT);
  dmx_set_pin(dmx_Port, dmx_transmitPin, dmx_receivePin, dmx_enablePin);
  
  
  switchValue |= digitalRead(pin_sw1) << 0;  // 将引脚12的值赋给变量的第0位
  switchValue |= digitalRead(pin_sw2) << 1;  // 将引脚14的值赋给变量的第1位
  switchValue |= digitalRead(pin_sw3) << 2;  // 将引脚25的值赋给变量的第2位
  switchValue |= digitalRead(pin_sw4) << 3;  // 将引脚26的值赋给变量的第3位
  switchValue=15-switchValue;   //拔码往上拔时，实际构成引脚接地为0。但拔码显示ON，所以采用取反处理
  /************以下这段代码只是专门给铁盒控制器临时使用  ***************/
  //switchValue=5;
  Serial.printf("switchValue is 0x%02X\n", switchValue);
}
  
void loop()
{

  switch(switchValue)
  {
    //DMX512通道测试程序
    case 0:               


      function_00(button0.numberKeyPresses);
      if (button0.numberKeyPresses>=function_00_mode_number)
        {button0.numberKeyPresses=0;}
      Serial.println(" 0000000000000000");
      delay(1000);
      break;

    //WS2811灯带测试程序
    case 1:                

      
      if(temp_01!=button0.numberKeyPresses)
        {
          function_01(button0.numberKeyPresses);
          //ws2812fx.setMode(button0.numberKeyPresses);
          Serial.print(F("Set mode to: "));
          Serial.print(ws2812fx.getMode());
          Serial.print(" - ");
          Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
          temp_01=button0.numberKeyPresses;
          delay(3000);
        }
      
      if(button0.numberKeyPresses>=function_01_mode_number){button0.numberKeyPresses=0;}
      ws2812fx.service();
      Serial.println(" 11111111111111");
      break;

    //手机控制DMX512
    case 2:
      //开启接收ARTNET信号，接收成功则调用回调函数
      if(temp_02!=button0.numberKeyPresses)
        {
          artnet.begin();
          artnet.setArtDmxCallback(onDmxFrame);
          temp_02=button0.numberKeyPresses;
          EEPROM_dmxData.begin(DMX_PACKET_SIZE-1);
          for (int i = 1; i < DMX_PACKET_SIZE; i++) 
            {
              dmxData[i] = EEPROM_dmxData.read(i);
              EEPROM_dmxData.commit();              
            }
          dmx_output(300000,0);
          delay(1000);
        }
      if(!artnet.read()){dmx_output(100000,1);};
      Serial.println(" 222222222222222");
      break;

    //APP/art-net/madrix调节WS2811像素点，每170点为一段，共6段，测过1020点是没问题的

    case 3:

      
      if(temp_03!=button0.numberKeyPresses)
        {
          artnet.begin();
          artnet.setArtDmxCallback(onDmxFrame);
          temp_03=button0.numberKeyPresses;
          Serial.println(" 33333333333333333");
        }
      if(!artnet.read())
        {
          set_adafruit_artnettopixel(dmxData,led_universe);
        }
     
      break;

    //APP控制WS2811变化模式
    case 4:

      if(temp_04!=button0.numberKeyPresses)
        {
          artnet.begin();
          artnet.setArtDmxCallback(onDmxFrame);
          temp_04=button0.numberKeyPresses;
          EEPROM_dmxData.begin(DMX_PACKET_SIZE-1);
          for (int i = 1; i < DMX_PACKET_SIZE; i++) 
            {
              dmxData[i] = EEPROM_dmxData.read(i);
              EEPROM_dmxData.commit();              
            }
          dmx_output(300000,0);
          delay(1000);
          Serial.println(" artnet begin ");

        }
      if(!artnet.read())
        {
          ////每5分钟保存一次DMX512通道值5*60*1000=300000
          dmx_save(100000);
          dmx_ws2812fx(dmxData);
         // Serial.println(" artnet reading ");
          
        }
      ws2812fx.service();
      //Serial.println(" 4444  444444444 44444444");
      break;


    
    //传感器低电平触发DMX512状态,典型应用：按键触发
    case 5:

    if(temp_04!=button0.numberKeyPresses)
        {
          //初始化传感器引脚4，
          pinMode(button4.PIN, INPUT_PULLUP);
          attachInterrupt(button4.PIN, isr_4, FALLING);
          temp_04=button0.numberKeyPresses;
          artnet.begin();
          artnet.setArtDmxCallback(onDmxFrame);
        }
        //dmx_output的参数决定了如果改变了通道值，必须按0键才能保存。
        //当然也可以设置一定时间保存，这样不必按0键，在断电下次上电的情况下，按4键便可以调出通道值。
      if(!artnet.read()){dmx_output(0,0);}   
      if(button4.pressed)
        {
          EEPROM_dmxData.begin(DMX_PACKET_SIZE-1);
          for (int i = 1; i < DMX_PACKET_SIZE; i++) 
            {
              dmxData[i] = EEPROM_dmxData.read(i);            
            }
          EEPROM_dmxData.commit();
          delay(200);  
          button4.pressed=false;
          button0.pressed=false;
        }

      if(button0.pressed)
        {
          dmx_output(1,1);
          delay(200);
          Serial.println(" button 0  pressedddddddddddddddddd0000   0000    0000   ");
          for (int i = 1; i < DMX_PACKET_SIZE; i++){dmxData[i]=0;}//第0值为0，
          button4.pressed=false;
          button0.pressed=false;
        }
      dmx_output(0,0);
      
 
      Serial.println(" 4444444444444");
      break;

    //传感器高电平触发DMX512状态,典型应用：微波传感器
    case 6:

      if(temp_05!=button0.numberKeyPresses)
        {
          //初始化传感器引脚4，
          pinMode(button4.PIN, INPUT_PULLDOWN);
          attachInterrupt(button4.PIN, isr_4, RISING);
          temp_05=button0.numberKeyPresses;
          artnet.begin();
          artnet.setArtDmxCallback(onDmxFrame);
        }
        //dmx_output的参数决定了如果改变了通道值，必须按0键才能保存。
        //当然也可以设置一定时间保存，这样不必按0键，在断电下次上电的情况下，按4键便可以调出通道值。
      if(!artnet.read()){dmx_output(0,0);}   
      if(button4.pressed)
        {
          EEPROM_dmxData.begin(DMX_PACKET_SIZE-1);
          for (int i = 1; i < DMX_PACKET_SIZE; i++) 
            {
              dmxData[i] = EEPROM_dmxData.read(i);            
            }
          EEPROM_dmxData.commit();
          delay(200);  
          button4.pressed=false;
          button0.pressed=false;
          Serial.println(" button 4 pressedddddddddddddddddd44 444 44 444 4444 4444   ");
        }

      if(button0.pressed)
        {
          dmx_output(1,1);
          delay(200);
          Serial.println(" button 0  pressedddddddddddddddddd0000   0000    0000   ");
          for (int i = 1; i < DMX_PACKET_SIZE; i++){dmxData[i]=0;}//第0值为0，
          button4.pressed=false;
          button0.pressed=false;
        }
      dmx_output(0,0);

      Serial.println("555555555555");
      break;
    //传感器低电平触发WS2811程序
    case 7:

      if(temp_07==0)
        {
          //初始化传感器引脚4，
          pinMode(button4.PIN, INPUT_PULLUP);
          //attachInterrupt(button4.PIN, isr_4, FALLING);
          temp_07=1;
          load_speed_ws2811();//读取掉电保存的速度
          Serial.printf("speed_ws2811 is 0x%02X\n", speed_ws2811);
          delay(2000);
          //artnet.begin();
          //artnet.setArtDmxCallback(onDmxFrame);
        }
        //dmx_output的参数决定了如果改变了通道值，必须按0键才能保存。
        //当然也可以设置一定时间保存，这样不必按0键，在断电下次上电的情况下，按4键便可以调出通道值。
      //if(!artnet.read()){dmx_output(1000,1);}   
      //button4是传感器引脚
      button4State = digitalRead(button4.PIN);
      
      if(button4State==1)
      //高电平，输出0，灭灯
        { 
          strip.clear();
          strip.show();
          function_07_mode_number=1;
        }
      else
      //低电平，
        {
          if(function_07_mode_number==1)
            {             
              strip.setBrightness(255); 
              //colorwipe的时序只针对COB白光，COB七彩灯带有效，对16703无效是因为接线的原因，可在后面加放大器
              colorWipe(strip.Color(0,   255,   0), speed_ws2811);
              function_07_mode_number=0;
              Serial.printf("speed_ws2811 is 0x%02X\n", speed_ws2811);
            }
          else
            {
              strip.setBrightness(255); 
              staticColor(strip.Color(0,255,0));
              Serial.printf(" keep lightingggggggggg    speed_ws2811 is  0x%02X\n", speed_ws2811);
            }
        }
      
      if(button0.pressed)
        {
          if(button0.numberKeyPresses>=11){button0.numberKeyPresses=0;}
          function_07_speedadjust(speed_ws2811,10);
          button0.pressed=false;
          Serial.printf("speed_ws2811 is        ssssssssssssssssssssssssssssssssssssssssss 0x%02X\n", speed_ws2811);
          delay(500);  
        }

      Serial.println(" 777777777777777777");
      break;


    //触摸传感器触发DMX512状态
    case 8:
      temp_08=10*touchRead(4);
      Serial.println(temp_08);  // get value using T1
      if(temp_08<255)
        {
          dmxData[1]=temp_08;
          dmx_output(100000,1);
          delay(200);
        }
      

      Serial.println(" 88888888888");
      
    break;
    case 9:
    Serial.println(" 9999999999999999");
    delay(1000);
    break;
    case 10:
    Serial.println(" 10,10,10,10,");
    delay(1000);
    break;
    case 11:
    Serial.println(" 11,11,11,11,11");
    delay(1000);
      //下面是H100作为DMX512有线解码器接控台控制灯带的代码，DMX512地址仍由上面无线设置，空间17，1，2通道和为解码器地址
      //下面的代码会阻塞，或者说即使没有收到信号，他也会占用很大一部分时间，导致ws2812fx.service运行不畅。
      if(!WiFi.isConnected()){
        WiFi.disconnect();
        }
      dmx_packet_t packet;
      if (dmx_receive(dmx_Port, &packet, DMX_TIMEOUT_TICK)) {
        Serial.println("receiving dmxdata******************************************* ");
        unsigned long now = millis();
        if (!packet.err) {
          if (!dmxIsConnected) {
            Serial.println("DMX is connected!");
            dmxIsConnected = true;
            }
          dmx_read(dmx_Port, dmxonlineData, packet.size);
          set_adafruit_rgbstatic(dmxonlineData);

          if (now - lastUpdate > 1000) {
            Serial.printf("Start code is 0x%02X and slot 1 is 0x%02X\n", dmxonlineData[0],dmxonlineData[1]);
            lastUpdate = now;
            }
        } 
        else {
          Serial.println("A DMX error occurred.");
          }
      }
    break;
    case 12:
    Serial.println(" 12,12,12,12,12");
    delay(1000);
    break;
    case 13:
    Serial.println(" 13,13,13,13,13");
    delay(1000);
    break;
    case 14:
    Serial.println("14,14,14,14,14,14");
    delay(1000);
    break;
    case 15:
    Serial.println(" 15,15,15,15,15,");
    delay(1000);
    break;
    default:
    break;
  }  

  //ws2812fx.service();
  //artnet.read();
  //delay(1);
 
}
  
