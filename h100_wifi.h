#ifndef Arduino_h
#include <Arduino.h>
#endif

#ifndef WiFi_h
#include <WiFi.h>
#endif

#ifndef EEPROM_h
#include <EEPROM.h> //导入Flash库文件
#endif

#ifndef h100_wifi_h
#define h100_wifi_h


extern const int flagPin;
extern short led_address;
extern EEPROMClass  EEPROM_WiFi;
extern EEPROMClass  EEPROM_dmxData;
extern EEPROMClass  EEPROM_led_address;
//下面是关于一键配网和ARTNET的设置
IPAddress AP_local_ip(192,168,4,1);          //IP地址
IPAddress AP_gateway(192,168,4,1);           //网关地址
IPAddress AP_subnet(255,255,255,0);       //子网掩码
const char* AP_ssid = "esp32_AP_H100";         //SSID
const char* AP_password = "12345678";     //密码
int ssid_psw_size=100;

//button0是按键，button21是传感器
//bool flag_smartconfigwifi=false;  //    false: 不一键配网  ture:  开始一键配网
extern bool flag_smartconfigwifi;


String ssid;
String psw;



//存取WIFI信息
struct config_type
{
  char stassid[32];//定义配网得到的WIFI名长度(最大32字节)
  char stapsw[64];//定义配网得到的WIFI密码长度(最大64字节)
};

config_type config;//声明定义内容

void saveConfig()//保存函数
{
 EEPROM_WiFi.begin(ssid_psw_size);//向系统申请100byte ROM
 //开始写入
 uint8_t *p = (uint8_t*)(&config);
  for (int i = 0; i < sizeof(config); i++)
  {
    EEPROM_WiFi.write(i, *(p + i)); //在闪存内模拟写入
  }
  EEPROM_WiFi.commit();//执行写入ROM
}

void loadConfig()//读取函数
{
  EEPROM_WiFi.begin(ssid_psw_size);
  uint8_t *p = (uint8_t*)(&config);
  for (int i = 0; i < sizeof(config); i++)
  {
    *(p + i) = EEPROM_WiFi.read(i);
  }
  EEPROM_WiFi.commit();
  ssid = config.stassid;
  psw = config.stapsw;
}

//  AP模式
void apConnect(void)
{
  //WiFi.disconnect(true, true);
  Serial.println("start to setup AP mode^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
  delay(1000);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(AP_local_ip, AP_gateway, AP_subnet);
  if (!WiFi.softAP(AP_ssid, AP_password)) {
    Serial.println("Soft AP creation failed.");
    delay(300);
  }
  Serial.println(" AP IP Address: ");
  delay(100);
  Serial.println(WiFi.softAPIP());
  delay(100);
  flag_smartconfigwifi=false;
}

void smartConfig()//配网函数
{
  
  Serial.println("start to smartconfig^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
  WiFi.stopSmartConfig();
  // 等待配网
  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
  int i=0;
  while(1)
  {
    if(i<60)   // 配网延时次数，
    {
      digitalWrite(flagPin, LOW);  //加个LED快闪，确认配网是否成功！成功就不闪了。
      delay(500);                    
      digitalWrite(flagPin, HIGH);   
      delay(500);
      if (WiFi.smartConfigDone())
        {
        strcpy(config.stassid,WiFi.SSID().c_str());//名称复制
        strcpy(config.stapsw,WiFi.psk().c_str());//密码复制
        saveConfig();//调用保存函数
        WiFi.setAutoConnect(true);  // 设置自动连接
        Serial.println("smartConfig done");
        flag_smartconfigwifi=false;
        break;
        }
      i++;
    }
    else{
      apConnect();
      break;
    }    
  }
}

// connect to wifi – returns true if successful or false if not
bool ConnectWifi(void)
{
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  loadConfig();//读取ROM是否包含密码
  int i=0;
  bool state=true;
  //判断ROM是否有密码
  if(ssid!=0&&psw!=0)
  {
    WiFi.begin(ssid,psw);//如果有密码则自动连接
    Serial.println(ssid);
    Serial.println(psw);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      if (i > 20){
        state = false;
        break;
        } 
      i++;
      }

    if (state){
      Serial.println("");
      Serial.print("Connected to ");
      Serial.println(ssid);
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      flag_smartconfigwifi=false;
      return true;
      } 
    else {
      return false;
      }    
  }
  return false;
}

void smartconfigafterconnectwifi()
{
    if(!ConnectWifi()){
    smartConfig();
  }
}
#endif