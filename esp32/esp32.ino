#include "esp_wpa2.h"
#include "MD_MAX72xx.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <time.h>

String webhook = "http://192.168.1.26:9999/";  // local proxy server for linebot
//String webhook = "http://10.40.251.31:9999/"; // local proxy server for linebot
const char *ssidap = "MAXBoard";                // SSID AP mode
const char *passwordap = "";                    // no password
  
// MD_MAX72xx hardware definitions and object
#define  MAX_DEVICES 16
#define CLK_PIN   14    
#define DATA_PIN  12    
#define CS_PIN    27    
MD_MAX72XX mx = MD_MAX72XX(DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);  
int TD_led_delay = 20;

// Global variables
char ssid[32];
char password[32];
char username[16];
char boardId[32];
int resetkey;                     // reset key
int resetIn = 2;                  // press button to select WIFI mode
String t="";                      // String for led display
unsigned long previousMillis = 0;        
unsigned long previousMillis_clock = 0;        
const long interval = 1000;       // the interval for webhook
const long interval_clock = 1000; // the interval for clock
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;
WebServer server(80);

void wifi( void * pvParamenters) {
    mx.begin();      // Max7219 begin
    // load data from EEPROM
    readEEPROM(0,32,ssid);
    readEEPROM(32,32,password);
    readEEPROM(64,10,username);
    readEEPROM(74,32,boardId);
    Serial.print("SSID=");Serial.println(ssid);
    Serial.print("Username=");Serial.println(username);
    Serial.print("Password=");Serial.println(password);
    printLedText("รอเชื่อมต่อ");

    if (strlen(username) > 0) {
        // EGATWIFI
        WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
        WiFi.mode(WIFI_STA);
        esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username)); //provide identity
        esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username)); //provide username
        esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password)); //provide password
        esp_wpa2_config_t config = WPA2_CONFIG_INIT_DEFAULT(); //set config to default (fixed for 2018 and Arduino 1.8.5+)
        esp_wifi_sta_wpa2_ent_enable(&config); //set config to enable function (fixed for 2018 and Arduino 1.8.5+)
        WiFi.begin(ssid); //connect to Wifi
        WiFi.setHostname("ESP32Maxboard"); //set Hostname for your device - not neccesary
        Serial.println("WPA2 Personel");
        Serial.print("SSID=");Serial.println(ssid);
        Serial.print("Password=");Serial.println(password);
    }
    else {
        // Home AP
        WiFi.begin(ssid, password);
    }
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    //init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    
    while(1) {
      if(WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi Disconnected.");
        WiFi.begin(ssid, password);
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      }
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    } 
}

void  showLED(void *pvParameters){
  mx.begin();      // Max7219 begin
  while(1) {
    if (t == "clock" || t == "") {
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo)){
          Serial.println("Failed to obtain time");
        }
        char timeStringBuff[50]; //50 chars should be enough
        strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);//print like "const char*"
        String clockStr =  String(timeStringBuff);
        Serial.print("led->");Serial.println(clockStr);
        printLedText(t);
    }
    else {
      Serial.print("led->");Serial.println(t);    
      printLedScroll(t);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
  }
}

void  getWebhook(void *pvParameters){
  HTTPClient http;
  while(1){
    http.begin(webhook+"kongdej.html?"+String(millis())); 
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    int httpCode = http.GET();                                  
    if (httpCode > 0) { 
        t = http.getString();
    }
    else {
      t="";
    }
    http.end();
    vTaskDelay(3000 / portTICK_PERIOD_MS); 
  }
}

void setup(){
  Serial.begin(115200);
  pinMode(resetIn, INPUT);
  resetkey = digitalRead(resetIn);                
  //resetkey = 0; //<---- force reset key
  Serial.print("mode:");Serial.println(resetkey);
  if (resetkey) {
    Serial.print("Setup Mode");
    xTaskCreate(&accessPoint, "accessPoint", 3000, NULL, 10, NULL);      
  }
  else {
    xTaskCreate(&getWebhook, "getWebhook", 3000, NULL, 9, NULL);
    xTaskCreate(&showLED, "showLED", 3000, NULL, 9, NULL);
    xTaskCreate(&wifi, "wifi", 3000, NULL, 10, NULL);
  }
}

void loop() {
  if (resetkey) {
    server.handleClient();
  }
}

/** Functions **/
/*============================== MAXBAORD THAI FONT ========================================*/
int g_statusCode;
int g_pcman_char = 0;
int g_pcman_n = 0;

int TD_max_row = 16;
int TD_max_col = (MAX_DEVICES/8)*32;
int TD_max_char_row1 = 10;
int TD_max_char_row2 = 12;
int TD_max_char_col = 16;
int TD_normal_row = 3;
int TD_start_not_thai_char = 33;
int TD_start_thai_char = 161;
int TD_gap_pixel = 1;

// thai over 
const int char_char_x1 = 13;
int char_x1[char_char_x1] = { 209,212,213,214,215,231,232,233,234,235,236,237,238 };

// thai under
const int char_char_x2 = 2;
int char_x2[char_char_x2] = { 216,217 };

unsigned int char_data1[93][10] = {
  {57344,57344,57344,57344,57344,57344,0,16384,57344,16384},         // 33 !
  {55296,55296,18432,18432,0,0,0,0,0,0},         // 34 "
  {0,2176,2176,32736,4352,4352,4352,65472,8704,8704},         // 35 #
  {8192,28672,43008,40960,24576,12288,10240,43008,28672,8192},         // 36 $
  {1536,1024,52224,51200,6144,4096,13824,9728,24576,49152},         // 37 %
  {0,28672,55296,34816,56064,29184,56320,35840,55808,29440},         // 38 &
  {49152,49152,49152,49152,0,0,0,0,0,0},         // 39 '
  {6144,12288,24576,49152,49152,49152,49152,24576,12288,6144},         // 40 (
  {49152,24576,12288,6144,6144,6144,6144,12288,24576,49152},         // 41 )
  {0,0,6144,23040,65280,32256,15360,32256,59136,16896},         // 42 *
  {0,0,6144,6144,6144,65280,65280,6144,6144,6144},         // 43 +
  {0,0,0,0,0,0,12288,28672,57344,49152},         // 44 ,
  {0,0,0,0,64512,64512,0,0,0,0},         // 45 -
  {0,0,0,0,0,0,24576,61440,61440,24576},         // 46 .
  {512,1536,3072,3072,6144,6144,12288,12288,24576,16384},         // 47 /
  {14336,27648,50688,50688,50688,50688,50688,50688,27648,14336},         // 48 0
  {24576,57344,57344,24576,24576,24576,24576,24576,61440,61440},         // 49 1
  {30720,64512,52224,3072,6144,12288,24576,49152,64512,64512},         // 50 2
  {30720,64512,52224,3072,14336,14336,3072,52224,64512,30720},         // 51 3
  {55296,55296,55296,55296,55296,55296,64512,64512,6144,6144},         // 52 4
  {64512,64512,49152,49152,63488,64512,3072,52224,64512,30720},         // 53 5
  {30720,64512,52224,49152,63488,64512,52224,52224,64512,30720},         // 54 6
  {64512,64512,3072,6144,12288,12288,12288,12288,12288,12288},         // 55 7
  {30720,64512,52224,52224,64512,64512,52224,52224,64512,30720},         // 56 8
  {30720,64512,52224,52224,64512,31744,3072,52224,64512,30720},         // 57 9
  {24576,61440,61440,24576,0,0,24576,61440,61440,24576},         // 58 :
  {24576,61440,61440,24576,0,28672,28672,61440,57344,49152},         // 59 ;
  {3072,6144,12288,24576,49152,49152,24576,12288,6144,3072},         // 60 <
  {0,0,0,64512,64512,0,64512,64512,0,0},         // 61 =
  {49152,24576,12288,6144,3072,3072,6144,12288,24576,49152},         // 62 >
  {30720,64512,52224,3072,6144,12288,12288,0,12288,12288},         // 63 ?
  {14336,17408,33280,37376,43520,43520,43520,37888,16384,14336},         // 64 @
  {6144,6144,15360,15360,9216,26112,26112,32256,49920,49920},         // 65 A
  {65024,49920,49920,49920,65024,49920,49920,49920,49920,65024},         // 66 B
  {15360,26112,49664,49152,49152,49152,49152,49664,26112,15360},         // 67 C
  {64512,50688,49920,49920,49920,49920,49920,49920,50688,64512},         // 68 D
  {65024,49152,49152,49152,65024,49152,49152,49152,49152,65024},         // 69 E
  {65024,49152,49152,49152,65024,49152,49152,49152,49152,49152},         // 70 F
  {15872,25344,49408,49152,49152,52992,49920,49920,25344,15616},         // 71 G
  {49920,49920,49920,49920,65280,49920,49920,49920,49920,49920},         // 72 H
  {49152,49152,49152,49152,49152,49152,49152,49152,49152,49152},         // 73 I
  {3072,3072,3072,3072,3072,3072,3072,52224,52224,30720},         // 74 J
  {50688,52224,55296,61440,61440,61440,55296,52224,50688,49920},         // 75 K
  {49152,49152,49152,49152,49152,49152,49152,49152,49152,65024},         // 76 L
  {49344,49344,57792,57792,62400,62400,57024,57024,52416,52416},         // 77 M
  {49920,58112,62208,62208,56064,56064,52992,52992,50944,49920},         // 78 N
  {15360,26112,49920,49920,49920,49920,49920,49920,26112,15360},         // 79 O
  {65024,49920,49920,49920,49920,65024,49152,49152,49152,49152},         // 80 P
  {15360,26112,49920,49920,49920,49920,49920,52992,26112,16128},         // 81 Q
  {65024,49920,49920,49920,49920,65024,49920,49920,49920,49536},         // 82 R
  {31744,50688,50688,49152,28672,7168,1536,50688,50688,31744},         // 83 S
  {65280,6144,6144,6144,6144,6144,6144,6144,6144,6144},         // 84 T
  {49920,49920,49920,49920,49920,49920,49920,49920,26112,15360},         // 85 U
  {49920,49920,26112,26112,26112,9216,15360,15360,6144,6144},         // 86 V
  {49932,49932,49932,26520,26520,14256,15600,6240,6240,6240},         // 87 W
  {49536,49536,25344,13824,7168,7168,13824,25344,49536,49536},         // 88 X
  {49344,49344,24960,13056,7680,3072,3072,3072,3072,3072},         // 89 Y
  {65408,384,768,1536,3072,6144,12288,24576,49152,65408},         // 90 Z
  {57344,32768,32768,32768,32768,32768,32768,32768,32768,57344},         // 91 [
  {32768,49152,16384,24576,8192,4096,6144,2048,3072,1024},         // 92 
  {57344,8192,8192,8192,8192,8192,8192,8192,8192,57344},         // 93 ]
  {4096,14336,27648,50688,0,0,0,0,0,0},         // 94 ^
  {0,0,0,0,0,0,0,0,65280,65280},         // 95 _
  {49152,24576,12288,6144,0,0,0,0,0,0},         // 96 `
  {0,0,31744,50688,512,32256,50688,33280,50944,32000},         // 97 a
  {49152,16384,16384,22528,25600,16896,16896,16896,25600,55296},         // 98 b
  {0,0,0,14336,17408,32768,32768,32768,17408,14336},         // 99 c
  {1536,1024,1024,13312,19456,33792,33792,33792,19456,13824},         // 100 d
  {0,0,0,30720,33792,33792,64512,32768,33792,30720},         // 101 e
  {7168,8704,8704,8192,63488,8192,8192,8192,8192,8192},         // 102 f
  {0,0,13312,19456,33792,33792,19456,13312,1024,14336},         // 103 g
  {32768,32768,32768,45056,51200,33792,33792,33792,33792,33792},         // 104 h
  {0,32768,0,32768,32768,32768,32768,32768,32768,32768},         // 105 i
  {0,4096,0,4096,4096,4096,4096,4096,36864,24576},         // 106 j
  {32768,32768,32768,34816,36864,40960,49152,40960,36864,38912},         // 107 k
  {0,32768,32768,32768,32768,32768,32768,32768,32768,32768},         // 108 l
  {0,0,0,46592,18688,18688,18688,18688,18688,18688},         // 109 m
  {0,0,0,47104,17408,17408,17408,17408,17408,17408},         // 110 n
  {0,0,0,30720,33792,33792,33792,33792,33792,30720},         // 111 o
  {0,0,63488,33792,33792,33792,63488,32768,32768,32768},         // 112 p
  {0,0,13312,19456,33792,33792,19456,13312,1024,3584},         // 113 q
  {0,0,0,45056,51200,32768,32768,32768,32768,32768},         // 114 r
  {0,0,0,28672,32768,32768,24576,4096,4096,57344},         // 115 s
  {16384,16384,61440,16384,16384,16384,16384,16384,18432,12288},         // 116 t
  {0,0,0,34816,34816,34816,34816,34816,35840,29696},         // 117 u
  {0,0,0,34816,34816,34816,55296,20480,28672,8192},         // 118 v
  {0,0,0,34944,34944,34944,56704,21760,30464,8704},         // 119 w
  {0,0,0,33280,17408,10240,4096,10240,17408,33280},         // 120 x
  {0,0,34816,34816,55296,20480,28672,8192,16384,32768},         // 121 y
  {0,0,0,64512,1024,2048,12288,16384,32768,64512},         // 122 z
  {6144,8192,8192,8192,57344,57344,8192,8192,8192,6144},         // 123 {
  {32768,32768,32768,32768,32768,32768,32768,32768,32768,32768},         // 124 |
  {49152,8192,8192,8192,14336,14336,8192,8192,8192,49152},         // 125 }
};

unsigned int char_data2[78][12] = {
  {0,0,0,64512,52224,19456,52224,52224,52224,52224,0,0},         // 161 ก
  //{0,0,0,64512,33792,17408,33792,33792,33792,33792,0,0},         // 161 ก
  {0,0,0,60416,60416,27648,27648,27648,27648,30720,0,0},         // 162 ข
  {0,0,0,44032,60416,27648,27648,27648,27648,30720,0,0},         // 163 ฃ
  {0,0,0,30720,52224,52224,60416,52224,52224,52224,0,0},         // 164 ค
  {0,0,0,52224,64512,52224,60416,52224,52224,52224,0,0},         // 165 ฅ
  {0,0,0,42496,58880,26112,26112,26112,30208,27648,0,0},         // 166 ฆ
  {0,0,0,7168,3072,52224,19456,27648,11264,15360,0,0},         // 167 ง
  {0,0,0,31744,50688,1536,7680,1536,1536,1536,0,0},         // 168 จ
  {0,0,0,30720,52224,3072,52224,52224,56320,27648,0,0},         // 169 ฉ
  {0,0,0,58880,27648,26112,26112,26112,26112,31744,0,0},         // 170 ช
  {0,0,0,42496,60416,26112,26112,26112,26112,31744,0,0},         // 171 ซ
  {0,0,0,63680,52416,19648,52416,52416,52928,60800,0,0},         // 172 ฌ
  {0,0,0,63680,52416,19648,52416,53184,53184,57344,3136,3968},         // 173 ญ
  {0,0,0,31744,26112,9728,26112,26112,26112,58880,1536,32256},         // 174 ฎ
  {0,0,0,31744,26112,9728,26112,26112,26112,58880,7680,28160},         // 175 ฏ
  {0,0,1536,32256,50688,1536,7680,1536,1536,1536,0,32256},         // 176 ฐ
  {0,0,0,48128,62976,26112,26112,26112,26112,26112,0,0},         // 177 ฑ
  {0,0,0,52416,64704,52416,52416,52416,61120,28032,0,0},         // 178 ฒ
  {0,0,0,63680,52416,19648,52416,52416,60864,59072,0,0},         // 179 ณ
  {0,0,0,31744,50688,50688,62976,54784,62976,26112,0,0},         // 180 ด
  {0,0,0,27648,65024,50688,62976,54784,62976,26112,0,0},         // 181 ต
  {0,0,0,63488,52224,19456,52224,52224,60416,60416,0,0},         // 182 ถ
  {0,0,0,60416,30208,26112,26112,26112,26112,26112,0,0},         // 183 ท
  {0,0,0,31744,49152,64512,3072,52224,52224,63488,0,0},         // 184 ธ
  {0,0,0,58880,26112,26112,26112,26112,28160,13824,0,0},         // 185 น
  {0,0,0,58880,26112,26112,26112,26112,26112,31744,0,0},         // 186 บ
  {0,1536,1536,58880,26112,26112,26112,26112,26112,31744,0,0},         // 187 ป
  {0,0,0,25344,49920,56064,56064,56064,56064,65024,0,0},         // 188 ผ
  {0,768,768,25344,49920,56064,56064,56064,56064,65024,0,0},         // 189 ฝ
  {0,0,0,60800,28032,28032,28032,28032,28032,32512,0,0},         // 190 พ
  {0,384,384,60800,28032,28032,28032,28032,28032,32512,0,0},         // 191 ฟ
  {0,0,0,31744,26112,9728,26112,26112,26112,58880,0,0},         // 192 ภ
  {0,0,0,58880,26112,26112,26112,26112,30208,27648,0,0},         // 193 ม
  {0,0,0,27648,52224,60416,27648,52224,52224,63488,0,0},         // 194 ย
  {0,0,0,30720,52224,49152,63488,3072,3072,6144,0,0},         // 195 ร
  {0,0,0,63488,52224,19456,52224,52224,60416,60416,3072,3072},         // 196 ฤ
  {0,0,0,30720,52224,3072,31744,52224,60416,60416,0,0},         // 197 ล
  {0,0,0,31744,26112,9728,26112,26112,26112,58880,1536,1536},         // 198 ฦ
  {0,0,0,28672,55296,55296,6144,6144,55296,28672,0,0},         // 199 ว
  {0,0,3072,31744,52224,52224,60416,52224,52224,52224,0,0},         // 200 ศ
  {0,0,0,58880,26112,26112,28416,26112,26112,31744,0,0},         // 201 ษ
  {0,0,3072,31744,52224,3072,31744,52224,60416,60416,0,0},         // 202 ส
  {0,0,0,60928,28160,30208,26112,26112,26112,26112,0,0},         // 203 ห
  {0,0,192,60800,28032,28032,28032,28032,28032,32512,0,0},         // 204 ฬ
  {0,0,0,30720,52224,3072,60416,52224,52224,64512,0,0},         // 205 อ
  {0,0,3072,31744,52224,3072,60416,52224,52224,64512,0,0},         // 206 ฮ
  {0,0,0,56320,64512,3072,3072,3072,3072,3072,0,0},         // 207 ฯ
  {0,0,0,0,49152,61440,0,49152,61440,0,0,0},         // 208 ะ
  {0,0,49152,63488,0,0,0,0,0,0,0,0},         // 209 ั
  {0,0,0,57344,12288,12288,12288,12288,12288,12288,0,0},         // 210 า
  {49152,49152,0,3584,768,768,768,768,768,768,0,0},         // 211 ำ
  {0,0,0,64512,0,0,0,0,0,0,0,0},         // 212 ิ
  {0,0,3072,64512,0,0,0,0,0,0,0,0},         // 213 ี
  {0,7168,5120,63488,0,0,0,0,0,0,0,0},         // 214 ึ
  {0,5120,5120,64512,0,0,0,0,0,0,0,0},         // 215 ื
  {0,57344,24576,24576,0,0,0,0,0,0,0,0},         // 216 ุ
  {0,60416,27648,31744,0,0,0,0,0,0,0,0},         // 217 ู
  {0,32768,0,0,0,0,0,0,0,0,0,0},         // 218 ฺ
  {8160,16368,25080,57852,65532,65532,65532,65532,65532,32760,16376,8167},         //  
  {8160,16368,25080,65532,2044,124,124,2044,65532,32761,16382,8160},         //  
  {}, 
  {}, 
  {}, 
  {0,0,0,49152,49152,49152,49152,49152,49152,49152,0,0},         // 224 เ
  {0,0,0,27648,27648,27648,27648,27648,27648,27648,0,0},         // 225 แ
  {28672,57344,24576,24576,24576,24576,24576,24576,24576,24576,0,0},         // 226 โ
  {24576,61440,45056,12288,12288,12288,12288,12288,12288,12288,0,0},         // 227 ใ
  {40960,61440,28672,12288,12288,12288,12288,12288,12288,12288,0,0},         // 228 ไ
  {0,0,0,57344,12288,12288,12288,12288,12288,12288,12288,12288},         // 229 ๅ
  {0,0,0,55296,64512,35840,52224,3072,3072,3072,3072,3072},         // 230 ๆ
  {1024,30720,50176,64512,0,0,0,0,0,0,0,0},         // 231 ็
  {12288,12288,0,0,0,0,0,0,0,0,0,0},         // 232 ่
  {13312,6144,0,0,0,0,0,0,0,0,0,0},         // 233 ้
  {29696,22528,0,0,0,0,0,0,0,0,0,0},         // 234 ๊
  {6144,15360,6144,0,0,0,0,0,0,0,0,0},         // 235 ๋
  {3072,6144,7168,0,0,0,0,0,0,0,0,0},         // 236 ์
  {3072,3072,0,0,0,0,0,0,0,0,0,0},         // 237 ํ
  {3072,3072,0,0,0,0,0,0,0,0,0,0},         // 238 ๎
};

void TD_setPoint(uint16_t r, uint16_t c, bool state)
{
  uint16_t nr = 0;
  uint16_t nc = 0;

  if (r<=0 || r>TD_max_row) return;
  if (c<=0 || c>TD_max_col) return;
  
  if (r>8) {
    r=r-8;
    nc=TD_max_col;
  }
  nr = r-1;
  nc = nc+TD_max_col-c;
  
  mx.MysetPoint(nr, nc, state);
}
char * TD_IntToBin(unsigned int p_ui)
{
  static char ret[17];        // need to change if expand font width

  for (int i=0; i<TD_max_char_col; i++) ret[TD_max_char_col-1-i]='0'+((p_ui&(1<<i))>0);
  ret[TD_max_char_col] ='\0';

  return ret;
}

void TD_SerialShowBinChar2(int p_idx)
{
  String c;

  Serial.println(p_idx);
  for (int i=0; i<TD_max_char_row2; i++) {
    c = TD_IntToBin(char_data2[p_idx][i]);
    c.replace("1","X");
    c.replace("0"," ");
    Serial.println(c);
  }    
  Serial.println("");
}    

int TD_IsX1(int idx)
{
  int ret=0;

  for (int i=0; i<char_char_x1; i++) {
    if (char_x1[i]==idx) {
      ret=1;
      break;
    }
  }

  return ret;
}

int TD_IsX2(int idx)
{
  int ret=0;

  for (int i=0; i<char_char_x2; i++) {
    if (char_x2[i]==idx) {
      ret=1;
      break;
    }
  }

  return ret;
}

void TD_WriteChar(int p_r, int p_c, int p_idx)
{
  char * c;
    
  if (p_idx==219) {
    if (g_pcman_n==0) {
      if (g_pcman_char==220) {
        g_pcman_char=219;
      } else {
        g_pcman_char=220;
      }
    }
    p_idx=g_pcman_char;
    g_pcman_n=g_pcman_n+1;
    if (g_pcman_n==8) g_pcman_n=0;
  }

  // not thai
  if (p_idx<TD_start_thai_char) {
    p_idx=p_idx-TD_start_not_thai_char;
    //TD_SerialShowBinChar2(p_idx);
    
    for (int i=0; i<TD_max_char_row1; i++) {
      c =  TD_IntToBin(char_data1[p_idx][i]);
      for (int j=0; j<TD_max_char_col; j++)
        if (c[j]=='1') {
          TD_setPoint(p_r+i, p_c+j, true);
        }
    }
  // thai
  } else {
    p_idx=p_idx-TD_start_thai_char;
    //TD_SerialShowBinChar2(p_idx);
    
    for (int i=0; i<TD_max_char_row2; i++) {
      c =  TD_IntToBin(char_data2[p_idx][i]);
      for (int j=0; j<TD_max_char_col; j++)
        if (c[j]=='1') {
          TD_setPoint(p_r+i, p_c+j, true);
        }
    }
  }

}

int TD_CharWidth(int p_idx)
{
  char * c;
  int w=0;

  if (p_idx<TD_start_thai_char) {
    p_idx=p_idx-TD_start_not_thai_char;

    for (int i=0; i<TD_max_char_row1; i++) {
      c =  TD_IntToBin(char_data1[p_idx][i]);
      for (int j=0; j<TD_max_char_col; j++)
        if (c[j]=='1') {
          if (j>=w) w=j;
        }
    }

  // thai
  } else {
    p_idx=p_idx-TD_start_thai_char;

    for (int i=0; i<TD_max_char_row2; i++) {
      c =  TD_IntToBin(char_data2[p_idx][i]);
      for (int j=0; j<TD_max_char_col; j++)
        if (c[j]=='1') {
          if (j>=w) w=j;
        }
    }
  }

  return w+1;
}

int TD_Char2Width(int p_idx)
{
  char * c;
  int w=0;

  for (int i=0; i<TD_max_char_row2; i++) {
    c =  TD_IntToBin(char_data2[p_idx][i]);
    for (int j=0; j<TD_max_char_col; j++)
      if (c[j]=='1') {
        if (j>=w) w=j;
      }
  }

  return w+1;
}

void TD_LEDWriteText(int p_r, int p_c, String p_text)
{
  char c1;
  char c2;
  int w=0;
  int idx=0;
  int normal;
  String myText;
  
  myText = p_text;
  mx.clear();  
  for (int i=0; i<myText.length(); i++) {
    c1 = myText[i];
    // E0 224 Thai
    if ((int)c1==224) {
      c1=myText[i+1];
      c2=myText[i+2];
      // B8,81(129)  ก - ฮ 
      if ((int)c1==184 && ((int)c2+32)>=161 && ((int)c2+32)<=207) {
        // ก 81 = 129 --> +32 --> 161
         idx=(int)c2+32;
      }
      // B8,B0(176)  สระ
      if ((int)c1==184 && ((int)c2+32)>=208 && ((int)c2+32)<=218) {
        // ะ B0 = 176 --> +32 --> 208
         idx=(int)c2+32;
      }
      // B9,80(128)  สระ
      if ((int)c1==185 && ((int)c2+96)>=224 && ((int)c2+96)<=231) {
        // เ 80 = 176 --> +96 --> 224
         idx=(int)c2+96;
      }
      // B9,B0(136)  วรรณยุก
      if ((int)c1==185 && ((int)c2+96)>=232 && ((int)c2+96)<=238) {
        // ่ 88 = 136 --> +96 --> 208
         idx=(int)c2+96;
      }
      i=i+2;
    } else {
      idx=(int)c1;
    }

    normal=1;
    // thai over
    if (TD_IsX1(idx)==1) {
      w=TD_CharWidth(idx);
      TD_WriteChar(p_r-2,p_c-(w+TD_gap_pixel),idx);
      normal=0;
    } 
    // thai under
    if (TD_IsX2(idx)==1) {
      w=TD_CharWidth(idx);
      TD_WriteChar(p_r+10,p_c-(w+TD_gap_pixel),idx);
      normal=0;
    } 
    if (normal==1) {
      switch (idx) {
        case 32:  // space
          p_c=p_c+8;
          break;
        case 211: // ำ
          w=TD_CharWidth(idx);
          TD_WriteChar(p_r,p_c-(3+TD_gap_pixel),idx);
          p_c=p_c-3+w+TD_gap_pixel;
          break;
        default:
          TD_WriteChar(p_r,p_c,idx);
          w=TD_CharWidth(idx);
          p_c=p_c+w+TD_gap_pixel;
          break;
      }
    }
  }
  mx.MyflushBufferAll();
  vTaskDelay(TD_led_delay / portTICK_PERIOD_MS); 
  //delay(TD_led_delay);
}

int TD_LEDTextPixel(String p_text)
{
  char c1;
  char c2;
  int w=0;
  int idx=0;
  int normal;
  String myText;
  int p_c;

  p_c=0;
  myText = p_text;
  for (int i=0; i<myText.length(); i++) {
    c1 = myText[i];
    // E0 224 Thai
    if ((int)c1==224) {
      c1=myText[i+1];
      c2=myText[i+2];
      // B8,81(129)  ก - ฮ 
      if ((int)c1==184 && ((int)c2+32)>=161 && ((int)c2+32)<=207) {
        // ก 81 = 129 --> +32 --> 161
         idx=(int)c2+32;
      }
      // B8,B0(176)  สระ
      if ((int)c1==184 && ((int)c2+32)>=208 && ((int)c2+32)<=218) {
        // ะ B0 = 176 --> +32 --> 208
         idx=(int)c2+32;
      }
      // B9,80(128)  สระ
      if ((int)c1==185 && ((int)c2+96)>=224 && ((int)c2+96)<=231) {
        // เ 80 = 176 --> +96 --> 224
         idx=(int)c2+96;
      }
      // B9,B0(136)  วรรณยุก
      if ((int)c1==185 && ((int)c2+96)>=232 && ((int)c2+96)<=238) {
        // ่ 88 = 136 --> +96 --> 208
         idx=(int)c2+96;
      }
      i=i+2;
    } else {
      idx=(int)c1;
    }

    normal=1;
    // thai over
    if (TD_IsX1(idx)==1) {
    } 
    // thai under
    if (TD_IsX2(idx)==1) {
      w=TD_CharWidth(idx);
      normal=0;
    } 
    if (normal==1) {
      switch (idx) {
        case 32:  // space
          p_c=p_c+8;
          break;
        case 211: // ำ
          w=TD_CharWidth(idx);
          p_c=p_c-3+w+TD_gap_pixel;
          break;
        default:
          w=TD_CharWidth(idx);
          p_c=p_c+w+TD_gap_pixel;
          break;
      }
    }
  }

  return p_c;
  
}

void TD_LEDScrollText(String p_text)
{
  int c;
  int n;

  n = TD_LEDTextPixel(p_text);
  for (int i=TD_max_col; i>=n*-1; i--) {
    TD_LEDWriteText(TD_normal_row,i,p_text);
  }
}  

void printLedText(String text) {
  TD_LEDWriteText(3,1,text);
}

void printLedScroll(String text){
  TD_LEDScrollText(text);
}

/*============================== MAXBAORD THAI FONT ========================================*/


/*= Read/Write EEPROM=*/
void writeEEPROM(int startAdr, int laenge, char* writeString) {
  EEPROM.begin(512); //Max bytes of eeprom to use
  yield();
  Serial.println();
  Serial.print("writing EEPROM: ");
  //write to eeprom 
  for (int i = 0; i < laenge; i++)
    {
      EEPROM.write(startAdr + i, writeString[i]);
      Serial.print(writeString[i]);
    }
  EEPROM.commit();
  EEPROM.end();           
}

void readEEPROM(int startAdr, int maxLength, char* dest) {
  EEPROM.begin(512);
  vTaskDelay(10 / portTICK_PERIOD_MS); 
  //delay(10);
  for (int i = 0; i < maxLength; i++)
    {
      dest[i] = char(EEPROM.read(startAdr + i));
    }
  EEPROM.end();    
  Serial.print("ready reading EEPROM:");
  Serial.println(dest);
}
  
/*==============================================================*/


/*== Web Server ==*/

void accessPoint(void *pvParameters) {
      mx.begin();
      Serial.println();
      Serial.println("Configuring access point...");

      WiFi.softAP(ssidap, passwordap);
      //delay(100);
      server.on("/", handleRoot);
      server.on("/setup", handleSetup);
      server.on("/list", handleList);
      server.begin();
      Serial.println("HTTP server started");
      while(1) {
       printLedScroll("SSID=MAXBoard , 192.168.4.1");
       vTaskDelay(1000 / portTICK_PERIOD_MS); 
     }
}

void handleRoot() {
  readEEPROM(0,32,ssid);
  readEEPROM(32,32,password);
  readEEPROM(64,10,username);
  readEEPROM(74,32,boardId);

  String content = "<html><head><title>MaxBoard</title>";
  content += "<style>";
  content += "form {border: 3px solid #f1f1f1; width:400px; margin: 30 auto;}";
  content += "input[type=text], input[type=password] {width: 100%;padding: 12px 20px;margin: 8px 0;display: inline-block;border: 1px solid #ccc;box-sizing: border-box;}";
  content += ".container {padding: 16px;}";
  content += "button {background-color: #4CAF50;color: white;padding: 14px 20px;margin: 8px 0;border: none;cursor: pointer;width: 100%;}button:hover {opacity: 0.8;}";
  content += "h1 {margin-top: 50px;text-align: center;}";
  content += "</style></head>";
  content += "<body>";
  content += "<h1>MaxBoard Configuration</h1>";
  content += "<form action='/setup' method='POST'>";
  content += "<div class='container'>";
  content += "<label><b>Board Name</b></label>";
  content += "<input type='text' placeholder='Enter Board Name' name='bname' value='"+String(boardId)+"'>";
  content += "<label><b>SSID</b></label>";
  content += "<input type='text' placeholder='WIFI SSID' name='ssid' value='"+String(ssid)+"'>";
  content += "<label><b>Username</b></label>";
  content += "<input type='text' placeholder='EGAT EMPN' name='username'  value='"+String(username)+"'>";
  content += "<label><b>Passsword</b></label>";
  content += "<input type='password' placeholder='WIFI Password' name='password'  value='"+String(password)+"'>";
  content += "<button type='submit'>OK</button>";
  content += "<p>&nbsp;&nbsp;&nbsp;&nbsp;Powered by EGAT MAKER CLUB</p></div></form> </body></html>";
  
  printLedText("รอข้อมูล");
  server.send(200, "text/html", content);
}

void handleList() {
  String content = "<html><head><title>MaxBoard</title>";
  
  // load data from EEPROM
  readEEPROM(0,32,ssid);
  readEEPROM(32,32,password);
  readEEPROM(64,10,username);
  readEEPROM(74,32,boardId);
  content += "Board Name = "+String(boardId)+"<br>";
  content += "SSID = "+String(ssid)+"<br>";
  content += "Username = "+String(username)+"<br>";
  content += "Password = "+String(password)+"<br>";
      
  server.send(200, "text/html", content);  
}

void handleSetup() {  
  if (server.hasArg("bname") && server.hasArg("ssid") && server.hasArg("password")){
    server.arg("ssid").toCharArray(ssid, 32); 
    server.arg("password").toCharArray(password, 32); 
    server.arg("username").toCharArray(username, 16); 
    server.arg("bname").toCharArray(boardId, 32);
     
    writeEEPROM(0,32, ssid);
    writeEEPROM(32,32, password);
    writeEEPROM(64,10, username);
    writeEEPROM(74,32, boardId);
    server.send(200,"text/html","<center><br>Setup Successful<br><br> <a href='/list'>List Settings</a></center>");
    Serial.println(ssid);
    Serial.println(username);
    Serial.println(password);
    Serial.println(boardId);
    printLedText("เก็บข้อมูล");
  }
  else {
    server.send(200,"text/html","Please enter all.<a href='/'>&lt;&lt; Back</a>");
  }
}

/*= End webserver =============*/
