#include <AceButton.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
//#include <stepperQ.h>
#include "BasicStepperDriver.h" // generic

#include "images.h"
//#include "frames.h"
//#include "fonts.h"
#include <Fonts/FreeSansBold24pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#include <FastLED.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266WebServerSecureAxTLS.h>
#include <ESP8266WebServerSecureBearSSL.h>
#include <ESP8266WiFi.h>        //Содержится в пакете. Видео с уроком http://esp8266-arduinoide.ru/step1-wifi
#include <ESP8266WebServer.h>   //Содержится в пакете. Видео с уроком http://esp8266-arduinoide.ru/step2-webserver
#include <FS.h>                 //Содержится в пакете. Видео с уроком http://esp8266-arduinoide.ru/step4-fswebserver
#include <ESP8266HTTPUpdateServer.h>  //Содержится в пакете.  Видео с уроком http://esp8266-arduinoide.ru/step8-timeupdate/
// Объект для обнавления с web страницы
ESP8266HTTPUpdateServer httpUpdater;
#include <ESP8266SSDP.h>        //Содержится в пакете. Видео с уроком http://esp8266-arduinoide.ru/step3-ssdp
#include <ArduinoJson.h> 

//_____________________________________________________________________________________________________________________________________
// Web интерфейс для устройства
ESP8266WebServer HTTP;
// Для файловой системы
File fsUploadFile;
WebSocketsServer webSocket = WebSocketsServer(81);

String configSetup = "{}";
String configJson = "{}";
String userJson = "{users:[]}";

int port = 80;


String utf8rus(String source);
using namespace ace_button;
#define OLED_RESET      LED_BUILTIN
#define LEFT_PIN        D5
#define OK_PIN          D6
#define RIGHT_PIN       D7
#define DIR_PIN         D4
#define STEP_PIN        D3
#define ENABLE_PIN      D0
#define DATA_PIN        8

#define NUM_LEDS        7


#define MICROSTEP       32
#define MAX_SPEED       400
#define ACCELERATION    3000

#define CONFIG_START    0
#define CONFIG_VERSION  "v02"

#define F_WAIT          0
#define F_FILL          99
#define F_MENU          1
#define F_ACTION        2
#define F_USERS         3
#define F_START         4

#define M_UP            1
#define M_DOWN          2
#define M_IN            3
#define M_OUT           4
#define M_INC           5
#define M_DEC           6


#define MENU_TOTAL      24

#define A_NOACTION      0
#define A_CHECK         1
#define A_DIGVAL        2
#define A_TEXT          3
#define A_ACTION        4
#define A_INFO          5

//unsigned char i1,i2,c3;
boolean redraw=true;
String inputString;
String curName; 
int curLimit = 35;
bool stringComplete = false;
int curMl=0, dispMl;
int curFrame=0;
int curMLevel=0;
int curMPos=1;
int curAPos=0;
int curUPos;
int totalUsers;
int curUsers[6];
int query[6];
int qPos;
bool isEditUser;
bool isOn;
bool isStart;
uint totalDrink=0;
int prevAPos[10];
int curDeep=0;
int yoff;
int doAction = 0;
int changingVal=0;
uint timer;
uint eyeTimer;
String curWifiPass;
String curWifiSsid;
//int cr=127,cg=127,cb=127;
boolean isArduino=false;
String IP = "192.168.11.1";
bool isWS=false;

struct MenuItem {
  int idx;
  int parent;
  String name;
  int action;
  const uint8_t *logo;
  int w;
  int h;
};

int curMenuItems[10];
int menuCounts;

MenuItem menu[MENU_TOTAL] = {
  {0  ,-1 ,utf8rus("Главное меню")        ,0        ,NULL   ,0  ,0},
  {1  ,0  ,utf8rus("Пользователи")        ,A_ACTION ,m1     ,24 ,30},
  {2  ,0  ,utf8rus("Настройки")           ,0        ,set_bmp     ,30 ,30},
  {3  ,2  ,utf8rus("WiFi")                ,0        ,NULL   ,0  ,0},
  {4  ,2  ,utf8rus("Калибровка")          ,0        ,NULL   ,0  ,0},
  {5  ,4  ,utf8rus("Головы")              ,A_ACTION ,NULL   ,0  ,0},
  {6  ,0  ,utf8rus("Старт")               ,A_ACTION ,start_bmp,30  ,30},
  {7  ,11 ,utf8rus("Шагов на 1 мл")       ,A_DIGVAL ,NULL   ,10 ,1000}, 
  {8  ,4  ,utf8rus("Вынос подачи")        ,0        ,NULL   ,0  ,0},
  {9  ,8  ,utf8rus("MAX")                 ,A_DIGVAL ,NULL   ,0  ,180},
  {10 ,8  ,utf8rus("MIN")                 ,A_DIGVAL ,NULL   ,0  ,180},
  {11 ,4  ,utf8rus("Насос")               ,0        ,NULL   ,0  ,0},
  {12 ,11 ,utf8rus("Налить 100 мл")       ,A_ACTION ,NULL   ,0  ,0},
  {13 ,11 ,utf8rus("Прокачка")            ,A_DIGVAL ,NULL   ,0  ,100},
  {14 ,3  ,utf8rus("Режим AP")            ,A_CHECK  ,NULL   ,0  ,0},
  {15 ,3  ,utf8rus("SSID")                ,A_TEXT   ,NULL   ,1  ,20},
  {16 ,3  ,utf8rus("Пароль")              ,A_INFO   ,NULL   ,8  ,20},
  {17 ,11 ,utf8rus("Откат")               ,A_DIGVAL ,NULL   ,0  ,50},
  {18 ,3  ,utf8rus("IP")                  ,A_INFO   ,NULL   ,0  ,50},
  {19 ,2  ,utf8rus("Режим")               ,0        ,NULL   ,0  ,50},
  {20 ,19 ,utf8rus("Рабочий")             ,A_ACTION ,NULL   ,0  ,50},  
  {21 ,19 ,utf8rus("Демо")                ,A_ACTION ,NULL   ,0  ,50},
  {22 ,2  ,utf8rus("Яркость LED")         ,A_DIGVAL ,NULL   ,0  ,254},
  {23 ,2  ,utf8rus("Сохранить")           ,A_ACTION ,NULL   ,0  ,50},          
};

struct settings {
  char version[4];
  String wifiSSID;
  String wifiPASS;
  String APSSID;
  String APPASS;  
  boolean isAP;
  int servoMAX;
  int servoMIN;
  int stepPerMl;
  int feedback;
  int startMl;
  uint8_t ledbright;  
};

settings option = {
  CONFIG_VERSION,
  "AlcoBot-WiFi",
  "12345678",
  "",
  "",
  true,
  100,
  10,
  10,
  5,
  50,
  128
};
int addr = 0;

struct user {
  String id;
  String name;
  String color;
  int doze;
  int total;
  int totalShots;
} users[6] /*= {
  {"ID1",utf8rus("Вася1"),25,0},
  {"ID2",utf8rus("2"),35,0},
  {"ID3",utf8rus("1234567890"),50,0},
  {"",utf8rus("Петя4"),15,0},
  {"ID5",utf8rus("Миша5"),40,0},
  {"ID6",utf8rus("Ваня6"),25,0},  
}*/;
struct DOCKS {
  int user;   //-1 - нет рюмки, 0-5 - номер юзера из users
  int state;  //-1 - нет рюмки, 0 - пусто, 1 - полный, 2 - новый
} dock[6] = {
  {-1,-1},
  {-1,-1},
  {-1,-1},
  {-1,-1},
  {-1,-1},
  {-1,-1}  
};
//user users[];

Adafruit_SSD1306 display(128,64,&Wire,OLED_RESET);
BasicStepperDriver stepper(200, DIR_PIN, STEP_PIN,ENABLE_PIN);
CRGB leds[NUM_LEDS];

ButtonConfig menuConfig;
AceButton bLeft(&menuConfig);
AceButton bRight(&menuConfig);
AceButton bOk(&menuConfig);

void handleMenuEvent(AceButton*, uint8_t, uint8_t);
void showFrame (int num);
void fLogo();
void serialEvent();
void onHold(int id);
void onClick(int id);
void onRepeat(int);
bool waitingA(String,String,int);
void sendCom(String,String);
void checkDocks();
void readCom();
void DSEvent(int,String);
void userNav(int);
void menuNav(int);
bool moveTo(int,String);
void fWait();
void fFill();
void fMenu();
void fAction();
void fUsers();
void drawText(String);
void drawSwitch(bool);
void drawChangeDig(String);
void userCount();
void docksDraw();
void pump(uint);
void eeLoad();
void eeSave();
void shotProgress(int,int,int);
void fStart();
void ledDraw(uint,CRGB);
void randEye();
void FS_init(void); 
String getContentType(String filename);
bool handleFileRead(String path);
void handleFileUpload();
void handleFileDelete();
void handleFileCreate();
void handleFileList();
void HTTP_init(void);
void SSDP_init(void);
String jsonRead(String &json, String name);
int jsonReadtoInt(String &json, String name);
bool jsonReadtoBool(String &json, String name);
String jsonWrite(String &json, String name, String volume);
String jsonWrite(String &json, String name, int volume);
String jsonWriteBool(String &json, String name, bool volume);
void saveConfig ();
String readFile(String fileName, size_t len );
String writeFile(String fileName, String strings );
void WIFIinit();
bool StartAPMode();
String findColor(String id);
void SocketData (String key, String data, String data_old);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void webSocket_init();
void SocketSend (String broadcast);
void SocketSendDock(int pos);
void startProc();

void setup(){                 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.cp437(true);
  fLogo();
  Serial.begin(9600);
  inputString.reserve(200);
  FS_init();
  configSetup = readFile("config.json", 4096);
  //Serial.println(configSetup);
  WIFIinit();
  HTTP_init();
  curName = utf8rus("Мастер Йода");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  menuConfig.setEventHandler(handleMenuEvent);
  menuConfig.setFeature(ButtonConfig::kFeatureClick);
  menuConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
  menuConfig.setFeature(ButtonConfig::kFeatureSuppressAfterClick);
  menuConfig.setFeature(ButtonConfig::kFeatureSuppressAfterRepeatPress);
  menuConfig.setFeature(ButtonConfig::kFeatureLongPress);
  menuConfig.setFeature(ButtonConfig::kFeatureSuppressAfterLongPress);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  bLeft.init(LEFT_PIN, HIGH, 0 /* id */);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  bRight.init(RIGHT_PIN, HIGH, 1 /* id */); 
  pinMode(OK_PIN, INPUT_PULLUP);
  bOk.init(OK_PIN, HIGH, 2 /* id */);

  stepper.begin(120,MICROSTEP);
  stepper.setSpeedProfile(stepper.LINEAR_SPEED, ACCELERATION, ACCELERATION);
  stepper.disable();
  webSocket_init();
  timer = millis();
  //display.clearDisplay();
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  //display.println("ON");
  while (!isArduino) {
    //readCom();
    //if (timer + 1000  < millis()) {
      //sendCom("INIT","ESP");
      if (waitingA("INIT","ARDU",3000)) {
        sendCom("ESP","0");
 //  display.print("-> ESP");
 //  display.display();        
        isArduino=true;
      }
      //timer = millis();
    //} 
  }
  display.setCursor(0,56);
  display.print("A - OK");
  display.display();
  display.setCursor(64,56);
  if (waitingA("READY","0",15000)) {
    display.print("Ready!");
    //delay(3000);
  } else {
    display.print("NOT ready!");
    //delay(3000);
  }
  timer=0;
  display.display();
  //eeLoad();
  delay(1000);
  //display.display();
  display.clearDisplay();
  display.display();  
  isOn=false;
  isStart=false; 
  FastLED.clear(); 
  FastLED.setBrightness(jsonReadtoInt(configSetup, "ledbright"));//option.ledbright);
  //option.wifiSSID = "AlcoBot";
  //option.wifiPASS = "12345678";
  eyeTimer=millis();

}

void loop() {
  bLeft.check();
  bRight.check();
  bOk.check();  
  //serialEvent();
  if (isOn && isStart) checkDocks();
  if (curFrame != F_FILL) redraw=true;
  showFrame(curFrame);
  //Serial.print("Ok");
  readCom();
  webSocket.loop(); // Работа webSocket
  // if (eyeTimer + 1000 < millis()) {
    randEye();
  //   eyeTimer = millis();
  // } 
  HTTP.handleClient();
  delay(50);

}


void eeLoad(){
  EEPROM.begin(512);
  //EEPROM.get(addr,option);
  //EEPROM.end();
  if (EEPROM.read(CONFIG_START + 0) == CONFIG_VERSION[0] &&
      EEPROM.read(CONFIG_START + 1) == CONFIG_VERSION[1] &&
      EEPROM.read(CONFIG_START + 2) == CONFIG_VERSION[2]) {
        for (unsigned int t=0; t<sizeof(option); t++)
        *((char*)&option + t) = EEPROM.read(CONFIG_START + t);
      } 
}
void eeSave(){
  EEPROM.begin(512);
  //EEPROM.put(addr,option);
  for (unsigned int t=0; t<sizeof(option); t++)
    EEPROM.write(CONFIG_START + t, *((char*)&option + t));
  EEPROM.commit();
  //EEPROM.end();
}
void readCom() {
  if(Serial.available() > 0) {  
        String str = Serial.readStringUntil('\n');
        char* frag1, *frag2;
        char buf[30];
        String v1,v2;
        str.toCharArray(buf,30);
        frag1 = strtok(buf,":");
        frag2 = strtok(NULL,":");
        if (frag1) v1=(String)frag1;
        if (frag2) v2=(String)frag2;
        v1.trim();
        v2.trim();
        //Serial.print("-> ");
        //Serial.println(v1+ " *** "+v2);
        if (v1 == "M9") {
          //option.servoMAX = v2.toInt();
          jsonWrite(configSetup,"servoMIN",v2.toInt());
          sendCom("M9","OK");
        }
        
        if (v1 == "M10") {
          //option.servoMIN = v2.toInt();
          jsonWrite(configSetup,"servoMIN",v2.toInt());
          sendCom("M10","OK");
        }

        if (v1.toInt()) {
          DSEvent(v1.toInt(), v2);
        }
        
      if (curFrame == F_ACTION) {
        if (v1== "M5" && v2 == "OK" && curMPos == 5) {
          doAction = 0;
          curFrame = F_MENU;          
        }

        if (v1== "M8" && v2 == "OK" && curMPos == 22) {
          doAction = 0;
          curFrame = F_MENU;          
        }
        if (v1== "M4" && v2 == "OK" && (curMPos == 20 || curMPos == 21)) {
          doAction = 0;
          curFrame = F_MENU;          
        }
      }
  }



}

void sendCom(String code, String state) {
  Serial.print(code);
  Serial.print(":");
  Serial.println(state);
}

bool waitingA (String code, String state, int time) {
  bool res=false;
  bool wait=true;
  if (state == "any") return true;
  uint t = millis();
  while (wait) {
    if(Serial.available() > 0) {
      //SocketSend("Serial");
      String instr = Serial.readStringUntil('\n');
      //SocketSend(instr);
      char* frag1, *frag2;
      char buf[30];
      String v1,v2;
      instr.toCharArray(buf,30);
      frag1 = strtok(buf,":");
      frag2 = strtok(NULL,":");
      if (frag1) v1=(String)frag1;
      if (frag2) v2=(String)frag2;
      v1.trim();
      v2.trim();
      //Serial.print("w-> ");
      // Serial.println(v1+ " *** "+v2);
      //SocketSend("v1: "+ String(v1) + " v2: " + v2 + " R: " + String(code) + " S: " + state);
      if (v1==code && v2==state) {
        //SocketSend("In code state");
        wait=false;
        res = true;      
      }
      if (v1.toInt() && state != "any") {
       //SocketSend("In DSEvent");
        DSEvent(v1.toInt(),v2);
        //SocketSend("out DSEvent");
      }
    }
    //SocketSend("W");
    if (t+time < millis()) {
      wait=false;
      //SocketSend("out time");
      }
  }
  //SocketSend("Ret: "+String(res));
  return res;
}
void checkDocks() {
  if (dock[qPos].user >= 0 && dock[qPos].state == 0 && users[dock[qPos].user].doze > 0) {
    if (moveTo(qPos, users[dock[qPos].user].id)) {
      //Serial.println("move ok");
      ledDraw(6, CRGB::Blue);
      dock[qPos].state = 1;
      SocketSendDock(qPos);
      sendCom("M2","0");
      delay(500);
      curName=utf8rus(users[dock[qPos].user].name);
      pump(users[dock[qPos].user].doze);
      delay(500);
      sendCom("M3","0");
      ledDraw(qPos,CRGB::Red);
      //ledDraw(6, CRGB::Aqua);
      totalDrink = totalDrink + users[dock[qPos].user].doze;
      users[dock[qPos].user].total = users[dock[qPos].user].total + users[dock[qPos].user].doze;
      users[dock[qPos].user].totalShots++;
      //Serial.println(users[dock[qPos].user].name);
    } else {
      //Serial.println("move BAD");
    }
  }
  qPos++;
  if (qPos>5) qPos=0;
}
bool moveTo(int pos, String code) {
  bool res = false;
  sendCom("M1",String(pos+1));

  if( waitingA("M1",String(pos+1), 5000)) {
    if (code != "menu"){
    sendCom("R",String(pos+1));
//SocketSend("P: "+ String(pos) + " C: " + code + " R: " + String(res));
    res = waitingA(String(pos+1),code, 1000);
    }
    if (code == "any" && pos == 0){
      res = true;
    }
  }
  //SocketSend("moveto ret: "+String(res));
  return res;
}
void pump(uint ml) {
  curMl=ml;
  fFill();
  int stepPerMl = jsonReadtoInt(configSetup,"stepPerMl");
  int feedback = jsonReadtoInt(configSetup,"feedback");
  stepper.enable();
  //ledDraw(6, CRGB::OrangeRed);
  stepper.move(ml*MICROSTEP*stepPerMl);
  stepper.move(-feedback *MICROSTEP*stepPerMl);
  stepper.disable();
  //ledDraw(6, CRGB::Aqua);
}
void ledDraw(uint led,CRGB color){
  leds[led]=color;
  FastLED.show();
}
void randEye() {
  static int r;
  static int g;
  static int b;
  static bool dirr;
  static bool dirg;
  static bool dirb;
  // leds[6].r = random(5,250);
  // leds[6].g = random(5,250);
  // leds[6].b = random(5,250);
  int shiftr = random(0,20);
  int shiftg = random(0,20);
  int shiftb = random(0,20);
  if (dirr) {
    r += shiftr;
  } else {
    r -= shiftr;
  }
  if (dirg) {
    g += shiftg;
  } else {
    g -= shiftg;
  }
  if (dirb) {
    b += shiftb;
  } else {
    b -= shiftb;
  } 
  if (r>240) dirr=false;
  if (g>240) dirg=false;
  if (b>240) dirb=false;
  if (r<15) dirr=true;
  if (g<15) dirg=true;
  if (b<15) dirb=true;
  leds[6].r=r;
  leds[6].g=g;
  leds[6].b=b;
  // int rand = random(-5,5);
  // int c;
  // c=cr;
  // c += rand;
  // if (c > 250 || c <10 ) c = cr;
  // cr = c;
  // leds[6].r = c;
  // rand = random(-5,5);
  // c = cg;
  // c += rand;
  // if (c > 250 || c <10 ) c = cg;
  // cg=c;
  // leds[6].g = c;
  // rand = random(-5,5);
  // c = cb;
  // c += rand;
  // if (c > 250 || c <10 ) c = cb;
  // cb=c;  
  // leds[6].b = c;  
  FastLED.show();
}
void handleMenuEvent(AceButton* button, uint8_t eventType,
    uint8_t /* buttonState */) {
  switch (eventType) {
    case AceButton::kEventReleased:
      onClick(button->getId());
      break;
    case AceButton::kEventLongPressed:
      onHold(button->getId());
      break;
    case AceButton::kEventRepeatPressed:
      onRepeat(button->getId());
      break;   
  }
}

void onRepeat(int id) {
  switch (id) {
    case 0:
      if(curFrame == F_FILL) curMl--;
      if(curFrame == F_MENU && doAction==2) changingVal--;
            
      break;
    case 1:
      if(curFrame == F_FILL) curMl++;
      if(curFrame == F_MENU && doAction==2) changingVal++; 
              
      break;
    default:
      break;
  }
  if (doAction == 2) {
    if (changingVal > menu[curMPos].h) changingVal=menu[curMPos].h;
    if (changingVal < menu[curMPos].w) changingVal=menu[curMPos].w;
  } 
  if (doAction == 2 && curMPos==9) {
    sendCom("M6",String(changingVal));
  }
  if (doAction == 2 && curMPos==10) {
    sendCom("M7",String(changingVal)); 
  } 
}

void onClick(int id) {
  switch (id) {
    case 0: //LEFT
      if(curFrame == F_FILL) curMl--;
      if(curFrame == F_MENU && !doAction) menuNav(M_DOWN);
      if(curFrame == F_MENU && doAction==2) changingVal--;
      if(curFrame == F_USERS) userNav(M_DOWN);
      if(curFrame == F_USERS && isEditUser) {
        changingVal=changingVal-5;
        if (changingVal<5) changingVal=5; 
      }
      break;
    case 1: //RIGHT
      if(curFrame == F_FILL) curMl++;
      if(curFrame == F_MENU && !doAction) menuNav(M_UP); 
      if(curFrame == F_MENU && doAction==2) changingVal++;
      if(curFrame == F_USERS) userNav(M_UP); 
      if(curFrame == F_USERS && isEditUser) {
        changingVal=changingVal+5;
        if (changingVal>50) changingVal=50;          
      }
      break;
    case 2: //OK  
      switch (curFrame) {
        case F_WAIT:
          //curFrame = F_FILL;
          break;
        case F_MENU:
        //Serial.print("onClick: ");
        //Serial.println(curMenuItems[curAPos]);
          if (curMenuItems[curAPos]==99) {
            menuNav(M_OUT);
          } else if (menu[curMPos].action) {
       //     Serial.println("ACTION");
            //curFrame=F_ACTION;
              switch (curMPos) {
                case 1:
                  curFrame = F_USERS;
                  userCount();
                  break;
                case 14:
                  //option.isAP = !option.isAP;
                  if (jsonReadtoBool(configSetup,"isAP")) {
                    jsonWriteBool(configJson,"isAP",false);
                  } else {
                    jsonWriteBool(configJson,"isAP",true);
                  }
                  break;
                case 5:
                  doAction = 4;
                  sendCom("M5","0"); 
                  curFrame = F_ACTION;
                  break;
                case 6:
                  curFrame = F_START;
                  break;  
                case 7:
                  if ( doAction==0) {
                    doAction=2;
                    //changingVal = option.stepPerMl;
                    changingVal = jsonReadtoInt(configSetup,"stepPerMl");
                  } else {
                    doAction = 0;
                    //option.stepPerMl=changingVal;
                    jsonWrite(configSetup,"stepPerMl",changingVal);
                  }
                  break;
                case 9:
                  if ( doAction==0) {
                    doAction=2;
                    //changingVal = option.servoMAX;
                    changingVal = jsonReadtoInt(configSetup,"servoMAX");
                  } else {
                    doAction = 0;
                    //option.servoMAX=changingVal;
                    jsonWrite(configSetup,"servoMAX",changingVal);
                  } 
                  break;
                case 10:
                  if ( doAction==0) {
                    doAction=2;
                    //changingVal = option.servoMIN;
                    changingVal = jsonReadtoInt(configSetup,"servoMIN");
                  } else {
                    doAction = 0;
                    //option.servoMIN=changingVal;
                    jsonWrite(configSetup,"servoMIN",changingVal);
                  } 
                  break;
                case 12:
                  //doAction = 4;
                  //sendCom("M5","0"); 
                  //curFrame = F_ACTION;
                  pump(100);
                  break;                                  
                case 13:
                  if ( doAction==0) {
                    doAction=2;
                    //changingVal = option.startMl;
                    changingVal = jsonReadtoInt(configSetup,"startMl");
                  } else {
                    doAction = 0;
                    //option.startMl=changingVal;
                    jsonWrite(configSetup,"startMl",changingVal);
                  }
                  break;    
                case 17:
                  if ( doAction==0) {
                    doAction=2;
                    //changingVal = option.feedback;
                    changingVal = jsonReadtoInt(configSetup,"feedback");
                  } else {
                    doAction = 0;
                    //option.feedback=changingVal;
                    jsonWrite(configSetup,"feedback",changingVal);
                  }
                  break; 
                case 20:
                  doAction = 4;
                  //Serial.println("M:0");
                  sendCom("M4","1"); 
                  curFrame = F_ACTION;
                  break;
                case 21:
                  doAction = 4;
                  //Serial.println("M:3");
                  sendCom("M4","3"); 
                  curFrame = F_ACTION;
                  break;
                case 22:
                  if ( doAction==0) {
                    doAction=2;
                    //changingVal = option.ledbright;
                    changingVal = jsonReadtoInt(configSetup,"ledbright");
                  } else {
                    doAction = 0;
                    //option.ledbright=changingVal;
                    jsonWrite(configSetup,"ledbright",changingVal);
                    FastLED.setBrightness(changingVal);
                  }
                  break;                   
                case 23:                  
                  doAction = 4;
                  //Serial.println("S3");
                  //eeSave();
                  saveConfig();
                  sendCom("M8","0"); 
                  curFrame = F_ACTION;
                  break;                                                                                                     
                default:
                break;
              }
          } else {
            menuNav(M_IN);
          }
          break;
        case F_FILL:
          curFrame = F_WAIT;
          break;
        case F_USERS:
          //userNav(M_IN);
          if (isEditUser) {
            userNav(M_OUT);
          } else {
            userNav(M_IN);
          }
          break;
        case F_START:
          startProc();
          break;
        default:
          break;
      }      
      break;
    default:
      break; 
  }
  if (doAction == 2) {
    if (changingVal > menu[curMPos].h) changingVal=menu[curMPos].h;
    if (changingVal < menu[curMPos].w) changingVal=menu[curMPos].w;
  }
  if (doAction == 2 && curMPos==9) {

    sendCom("M6",String(changingVal)); 
  }
  if (doAction == 2 && curMPos==10) {

    sendCom("M7",String(changingVal)); 
  } 
    if (doAction == 2 && curMPos==22) {
      FastLED.setBrightness(changingVal);
  }  
}

void startProc() {
            if (!isStart) {
            if (moveTo(0,"any")) {
              sendCom("M2","0");
              delay(500);
              //pump(option.startMl);
              pump(jsonReadtoInt(configSetup,"startMl"));
              delay(500);
              sendCom("M3","0");
              isStart=true;
              curFrame=F_WAIT;
            }
          } else {
            curFrame = F_MENU;
          }
}

void onHold(int id) {
  switch (id) {
    case 0:  //LEFT
      break;
    case 1:  //RIGHT
      break;
    case 2:  //OK  
      switch (curFrame) {
        case F_WAIT:
          curFrame = F_MENU;
          moveTo(0,"menu");
          break;
        case F_MENU:
          curFrame = F_WAIT;
          break;
        case F_FILL:
          curFrame = F_MENU;
          break;
        case F_USERS:
          curFrame = F_MENU;
          break;          
        default:
          break;
      }
      break;
    default:
      break; 
  }
}

void showFrame (int num) {
  curFrame = num;
  switch (num) {
    case F_WAIT:
      isOn = true;
      fWait();
      break;    
    case F_FILL:
      fFill();
      break;
    case F_MENU:
      isOn = false;
      fMenu();
      break;
    case F_ACTION:
      fAction(); 
      break; 
    case F_USERS:
      fUsers(); 
      break;
    case F_START:
      fStart(); 
      break;                  
    default:
    display.clearDisplay();
    display.display();
    break;
  }
}


void shotProgress (int ml, int x, int y) {
  int h=18, startx=0, starty;
  //u8g2.drawXBMP( x, y, shot_width, shot_height, shot_bits);
  for (int i= 0 ; i <= ml; i++) {
    //if (i=0) {}
    if (i>0 && i<=3) {
      h=18;
      startx=x+9;
    }
    if (i>3 && i<=14) {
      h=20;
      startx=x+8;
    }
    if (i>14 && i<=25) {
      h=22;
      startx=x+7;
    }
    if (i>25 && i<=36) {
      h=24;
      startx=x+6;
    }
    if (i>36 && i<=47) {
      h=26;
      startx=x+5;
    }
    if (i>47 && i<=50) {
      h=28;
      startx=x+4;
    }
    starty = 57 - i;
    if (i!=0) {display.drawFastHLine(startx, starty, h,WHITE);}
  }

  //if (ml == 0 ) {
  //u8g2.setBitmapMode(true /* transparent*/);
  //display.drawXBitmap( x, y, shot_bits,shot_width, shot_height, WHITE);
  //}
}

void TEST_display_4()
{
  display.clearDisplay(); 
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  display.setTextSize(1);
  display.println(utf8rus("Размер шрифта 1"));

  display.setTextSize(2);
  display.println(utf8rus("Размер 2"));

  display.setTextSize(3);
  display.println(utf8rus("Разм 3"));

  display.display();  
}

/* Recode russian fonts from UTF-8 to Windows-1251 */
String utf8rus(String source)
{
  int i,k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); i = 0;

  while (i < k) {
    n = source[i]; i++;

    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; target = target + String(m);
  }
return target;
}

void menuCount () {
  int x=0, i=0;
  while (i < MENU_TOTAL) {
    if (curMLevel == menu[i].parent) {
      curMenuItems[x] = menu[i].idx;
      x++;
    }
    i++;
  }
  menuCounts=x;
  if (curMLevel != 0) {
    curMenuItems[menuCounts]=99;  //Добавили НАЗАД в список подменю начиная с 1 уровня
    menuCounts++;
  }      
}
void menuNav(int action) {
  menuCount();
  int tempL=curMLevel;
  switch(action) {
    case M_UP:
      curAPos++;
      if (curAPos > (menuCounts-1)) curAPos=0;
      curMPos = curMenuItems[curAPos];
      break;
    case M_DOWN:
      curAPos--;
      if (curAPos < 0) curAPos=menuCounts-1; 
      curMPos = curMenuItems[curAPos];      
      break;
    case M_IN:
      curMLevel = curMPos;
      curAPos=0;
      menuCount();
      curMPos = curMenuItems[0];
      break;
    case M_OUT:
      curMLevel=menu[curMLevel].parent;
      menuCount();      
      for (int i = 0; i < menuCounts; i++) {
        if (curMenuItems[i] == tempL) { 
          curAPos=i;
          //if (curMLevel == 0) 
          curMPos = curMenuItems[curAPos];
          break;
        }
      } 
    //Serial.print("curMPose: ");
    //Serial.println(curMPos);
      break;
    case M_INC:
      break;
    case M_DEC:
      break;
    default:
      break;      
  }
}

//      FRAMES

// LOGO display
void fLogo()
{
  display.clearDisplay();
  display.drawBitmap(0, 0, AlcoBot_Logo_FS, 128, 64, WHITE);
  display.display();
}
//  Menu frames
void fMenu() {
  display.clearDisplay();
  //display.setTextSize(1);
  // MAIN MENU
  if (curMLevel == 0) {
    display.setCursor(1,1);
    display.setTextColor(WHITE);
    display.print(menu[curMLevel].name);
    display.drawFastHLine(0,10, 128, WHITE);
    int xx=(128-menu[curMPos].w)/2;
    int yy=(64-menu[curMPos].h)/2;
    display.drawBitmap(xx, yy, menu[curMPos].logo, menu[curMPos].w, menu[curMPos].h, WHITE);
    int16_t  x1, y1;
    uint16_t w, h;
    display.getTextBounds(menu[curMPos].name, 0, 55, &x1, &y1, &w, &h);
    display.setCursor((128-w)/2,55);
    display.print(menu[curMPos].name);
  }
  //MAIN MENU -----END

  //SUB MENU
  if (curMLevel != 0) {
    display.setCursor(1,1);
    display.setTextColor(WHITE);
    display.print(menu[curMLevel].name);
    display.drawFastHLine(0,10, 128, WHITE);
    yoff = 13;
    for (int i = 0; i < menuCounts; i++) {
      if (curMenuItems[i] != 99) {
        display.setCursor(5,yoff);
        display.print(menu[curMenuItems[i]].name);
        if (menu[curMenuItems[i]].action == A_ACTION) {
          display.fillTriangle(110, yoff, 110, yoff+8, 128, yoff+4, WHITE);
        }
        switch (curMenuItems[i]) {
          case 7:
            //drawText(String(option.stepPerMl));
            drawText(String(jsonReadtoInt(configSetup,"stepPerMl")));
          break;         
          case 9:
            //drawText(String(option.servoMAX));
            drawText(String(jsonReadtoInt(configSetup,"servoMAX")));
          break;  
          case 10:
            //drawText(String(option.servoMIN));
            drawText(String(jsonReadtoInt(configSetup,"servoMIN")));
          break; 
          case 13:
            //drawText(String(option.startMl)+" ml");
            drawText(String(jsonReadtoInt(configSetup,"startMl"))+ " ml");
          break;                          
          case 14:
            //drawSwitch(option.isAP);
            drawSwitch(jsonReadtoBool(configSetup,"isAP"));
          break;
          case 15:
            drawText(curWifiSsid);
            break;
          case 16:
            drawText(curWifiPass);//wifi pass
          break;
          case 17:
            //drawText(String(option.feedback)+" ml");
            drawText(String(jsonReadtoInt(configSetup,"feedback"))+ " ml");
          break;          
          case 18:
            drawText("ToDo");//ip
           break;           
          default:
          break;
          
        }
        yoff=yoff+9;
      } else {
        display.setCursor(5,yoff);
        display.print(utf8rus("Назад"));
      }
    }

    //if (doAction == 2 && curMPos == 7) drawChangeDig(menu[curMPos].name, "");
    //if (doAction == 2 && curMPos == 13) drawChangeDig(menu[curMPos].name, " ml");
    if (doAction == 2 ) drawChangeDig(menu[curMPos].name);
    display.fillRect(0,12+(9*curAPos),4,9,WHITE);
  }
  
  display.display();
}

void drawChangeDig (String cap) {
  display.fillRoundRect(16, 15, 96, 34,5, BLACK);
  display.drawRoundRect(18, 17, 92, 30,5, WHITE);
  uint16_t w, h;
  int16_t  x1, y1;
  display.getTextBounds(cap, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128-w)/2,22);
  display.print(cap);
  String suff="";
  if (curMPos == 17 || curMPos == 13) suff = " ml"; 
  display.getTextBounds(String(changingVal)+suff, 0, 0, &x1, &y1, &w, &h);  
  display.setCursor((128-w)/2,33);
  display.print(changingVal+suff);
  display.fillTriangle(25,33,25,41,20,37,WHITE);  
  display.fillTriangle(102,33,102,41,107,37,WHITE);   
}

void drawSwitch(bool state){
  if (state) {
    display.drawRoundRect(110, yoff, 18, 9, 4, WHITE);
    display.fillCircle(122, yoff+4, 2, WHITE);
  } else {
    display.drawRoundRect(110, yoff, 18, 9, 4, WHITE);
    display.drawCircle(114, yoff+4, 2, WHITE);
  }
}

void drawText(String str) {
  uint16_t w, h;
  int16_t  x1, y1;
  display.getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128-w),yoff);
  display.print(str);
  
}

//Shot filling frame
void fFill()  {
  if (dispMl > curMl) redraw=true;
  if (curMl >50 || curMl<0) curMl=dispMl;
  dispMl=curMl;
  if (redraw) {
      display.clearDisplay();
      display.drawRect(0, 0, 89, 12, WHITE);
      display.drawRect(0, 14, 89, 50, WHITE); 
      display.drawXBitmap( 91, 3, shot_bits,shot_width, shot_height, WHITE);
      display.setFont();
      //display.setTextSize(1);
      display.setCursor(5,3);
      display.setTextColor(WHITE);
      display.print(curName);
      redraw=false;
    }
    display.fillRect(1, 15, 87, 47, BLACK),
    display.setFont(&FreeSansBold24pt7b);
    // //display.setTextSize(1);
    display.setCursor(5,55);
    display.setTextColor(WHITE);
    display.print(curMl);
    display.setFont(&FreeSans12pt7b);
    display.print("ml");
    shotProgress(curMl,91,3);
    display.display();
    display.setFont();
}
// Wait frame
void fWait() {
  display.clearDisplay();
  docksDraw();
  //display.setTextSize(1);
  display.setCursor(10,20);
  display.setTextColor(WHITE);
  display.println("Waiting...");
  if (isWS) display.println("WebSocket ON...");
  /*  // Вывод LED на экран
  String rs,gs,bs;
  for (int i=0; i<6; i++ ){
    rs += String(leds[i].r);
    rs +=" ";
    gs += String(leds[i].g);
    gs +=" "; 
    bs += String(leds[i].b);
    bs +=" ";       
  }
  display.println("R: "+ String(rs));
  display.println("G: "+ String(gs));
  display.println("B: "+ String(bs));*/
  display.display();
}
void docksDraw() {
  int x = 4;
  int y = 4;
  int shift = 10;
  for (int i = 0; i < 6; i++){
    if (dock[i].user == -1) {
      display.drawCircle(x, y, 1, WHITE);
      //ledDraw(i,CRGB::Black);
    }
    if (dock[i].user != -1 && dock[i].state == 0) {
      display.drawCircle(x, y, 4, WHITE);
      //ledDraw(i,CRGB::Green);
      }
    if (dock[i].user != -1 && dock[i].state == 1) {
      display.fillCircle(x, y, 4, WHITE);
      //ledDraw(i,CRGB::Red);
      }
    if (dock[i].user != -1 && dock[i].state == 2) {
      display.fillCircle(x, y, 2, WHITE);
      //ledDraw(i,CRGB::Yellow);
      }
    x = x + shift;
  }
  
  display.drawBitmap( 70, 0, drop,7, 9, WHITE);
  display.setCursor(82,2);
  display.setTextColor(WHITE);
  display.print(String(totalDrink)+" ml");
}
void fAction() {
  //  display.setCursor(1,1);
  //  display.setTextColor(WHITE);
  //  display.print(menu[curMPos].name);
  //  display.drawFastHLine(0,10, 128, WHITE);
  String cap;
  if (!timer) {
    timer = millis();
  }
  if (timer+5000 < millis()) {
    curFrame=F_MENU;
    doAction=0;
    timer=0;
  }
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64,5, WHITE);
  switch (curMPos) {
    case 5:  //Калибровка головы
        cap = utf8rus("Калибровка головы");
      break;
    case 20:  //Режим - норма
        cap = utf8rus("Режим - НОРМА");
      break;
    case 21:  //Режим - демо
        cap = utf8rus("Режим - ДЕМО");
      break;
    case 22:  //Сохранить настройки сервы
        cap = utf8rus("Сохранение настроек");
      break;      
    default:
    break;
  }
  uint16_t w, h;
  int16_t  x1, y1;
  display.getTextBounds(cap, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128-w)/2,22);
  display.print(cap);
  display.display();
}

void userCount() {
  totalUsers=0;
  for (int i = 0; i<6; i++) {
    if (users[i].id.length() > 2) {
      curUsers[totalUsers]=i;
      //Serial.println(users[i].id);
      totalUsers++;
    }
  }
}

void userNav(int action) {
  userCount();
  switch(action) {
    case M_UP:
      if(!isEditUser) curUPos++;
      if (curUPos > totalUsers-1) curUPos=0;
      break;
    case M_DOWN:
      if(!isEditUser) curUPos--;
      if (curUPos < 0 ) curUPos=totalUsers-1;
      break;
    case M_IN:
      //Serial.println("M-IN");
      isEditUser = true;
      changingVal = users[curUsers[curUPos]].doze;
      break;
    case M_OUT:
      //Serial.println("M-OUT");
      isEditUser = false;
      users[curUsers[curUPos]].doze = changingVal;
      break;
  
  }
}

void DSEvent(int pos, String code) {//pos 1-6
  bool isNew = true;
  pos = pos-1;                      //pos 0-5
 
  //SocketData("pos",String(pos),"99");
  if (code == "0") {
//SocketSend("id: "+users[dock[pos].user].id+" doze: "+ String(users[dock[pos].user].doze));
    if (users[dock[pos].user].id.length() > 2 && users[dock[pos].user].doze == 0) {
      //SocketSend("delete");
      int del = dock[pos].user;
            users[del].id = "";
            users[del].name = "";
            users[del].doze = 0;
            users[del].color = "";
            users[del].total = 0;
            users[del].totalShots = 0;
    }
    dock[pos].user = -1;
    dock[pos].state = -1;
    ledDraw(pos,CRGB::Black);
    
  } else {
    userCount();
    for (int i = 0 ; i<6 ; i++) {
      if (users[i].id == code) {
        isNew = false;
        dock[pos].user = i;
        if (users[i].doze > 0) {
          dock[pos].state = 0; 
          ledDraw(pos,CRGB::Green);
        }
        if (users[i].doze == 0) dock[pos].state = 2; 
      } 
    }
    if (isNew) {
        String colorName =findColor(code);
        users[totalUsers].id = code;
        users[totalUsers].name = colorName;//totalUsers+1;
        users[totalUsers].color = colorName;
        users[totalUsers].doze = 0;
        users[totalUsers].total = 0;
        dock[pos].user=totalUsers;
        dock[pos].state=2; 
        ledDraw(pos,CRGB::Yellow);
        totalUsers++;  
    }
  } 
  SocketSendDock(pos);
}

void fUsers(){

    String str ="";
    display.clearDisplay();
    display.setCursor(5,0);
    display.print(utf8rus("Список джедаев"));
    display.drawFastHLine(0,9, 128, WHITE);
    for (int i = 0; i < totalUsers; i++) {
        display.setCursor(5,11+(i*9));
        String pos="-";
        for ( int x = 0 ; x <  6;x++) {
          if (dock[x].user == i)  pos = String (x+1);
        }
        str = pos + "|" + utf8rus(users[curUsers[i]].name); 
        display.print(str);
        display.setCursor(75,11+(i*9));
        str = "|" + String(users[curUsers[i]].doze) + "|" + String(users[curUsers[i]].total);
        display.print(str);  
     
    }
    display.fillRect(0,10+(9*curUPos),4,9,WHITE);
    if (isEditUser)  {
        display.fillRoundRect(8, 15, 112, 34,5, BLACK);
        display.drawRoundRect(10, 17, 108, 40,5, WHITE);
        uint16_t w, h;
        int16_t  x1, y1;
        display.getTextBounds(utf8rus(users[curUsers[curUPos]].name), 0, 0, &x1, &y1, &w, &h);
        display.setCursor((128-w)/2,22);
        display.print(utf8rus(users[curUsers[curUPos]].name));
        display.getTextBounds(utf8rus("Доза: ")+String(changingVal)+" ml", 0, 0, &x1, &y1, &w, &h);  
        display.setCursor((128-w)/2,33);
        display.print(utf8rus("Доза: ")+String(changingVal)+" ml");
        display.setCursor(13,43);
        display.print("ID:"+users[curUsers[curUPos]].id);
        display.fillTriangle(25,33,25,41,20,37,WHITE);  
        display.fillTriangle(102,33,102,41,107,37,WHITE);
    }
    display.display();
}

void fStart(){
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64,5, WHITE);
  uint16_t w, h;
  int16_t  x1, y1;
  display.getTextBounds(utf8rus("Подготовка к работе"), 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128-w)/2,2);
  display.print(utf8rus("Подготовка к работе"));
  display.drawFastHLine(0,12,128,WHITE);
  display.setCursor(3,20);
  if (!isStart) {
    display.print(utf8rus(" Установите посуду   объемом не менее 50  мл в первый слот и   нажмите ОК"));
  }  else { display.print(utf8rus(" Система прокачана,  подготовка не        требуется"));}
  //display.print(utf8rus("Установите посуду    объемом не менее 50  мл в первый слот и   нажмите среднюю ОК"));
  display.display();

}


void FS_init(void) {
  SPIFFS.begin();
  
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
    }
  }
  
  //HTTP страницы для работы с FFS
  //list directory
  HTTP.on("/list", HTTP_GET, handleFileList);
  //загрузка редактора editor
  HTTP.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) HTTP.send(404, "text/plain", "FileNotFound");
  });
  //Создание файла
  HTTP.on("/edit", HTTP_PUT, handleFileCreate);
  //Удаление файла
  HTTP.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  HTTP.on("/edit", HTTP_POST, []() {
    HTTP.send(200, "text/plain", "");
  }, handleFileUpload);
  //called when the url is not defined here
  //use it to load content from SPIFFS
  HTTP.onNotFound([]() {
    if (!handleFileRead(HTTP.uri()))
      HTTP.send(404, "text/plain", "FileNotFound");
  });
}
// Здесь функции для работы с файловой системой
String getContentType(String filename) {
  if (HTTP.hasArg("download")) return "application/octet-stream";
  else if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".json")) return "application/json";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path) {
  if (path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = HTTP.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (HTTP.uri() != "/edit") return;
  HTTPUpload& upload = HTTP.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile)
      fsUploadFile.close();
  }
}

void handleFileDelete() {
  if (HTTP.args() == 0) return HTTP.send(500, "text/plain", "BAD ARGS");
  String path = HTTP.arg(0);
  if (path == "/")
    return HTTP.send(500, "text/plain", "BAD PATH");
  if (!SPIFFS.exists(path))
    return HTTP.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  HTTP.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (HTTP.args() == 0)
    return HTTP.send(500, "text/plain", "BAD ARGS");
  String path = HTTP.arg(0);
  if (path == "/")
    return HTTP.send(500, "text/plain", "BAD PATH");
  if (SPIFFS.exists(path))
    return HTTP.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if (file)
    file.close();
  else
    return HTTP.send(500, "text/plain", "CREATE FAILED");
  HTTP.send(200, "text/plain", "");
  path = String();

}

void handleFileList() {
  if (!HTTP.hasArg("dir")) {
    HTTP.send(500, "text/plain", "BAD ARGS");
    return;
  }
  String path = HTTP.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();
  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  output += "]";
  HTTP.send(200, "text/json", output);
}

void HTTP_init(void) {
  

  // -------------------Выдаем данные configSetup
  HTTP.on("/config.json", HTTP_GET, []() {
    HTTP.send(200, "application/json", configSetup);
  });
  HTTP.on("/start", HTTP_GET, []() {
    startProc();
    HTTP.send(200, "text/plain", "OK");
  });

  // -------------------Выдаем данные users
  HTTP.on("/users.json", HTTP_GET, []() {

    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& usersNode = root.createNestedArray("users");
    for (int i=0; i < totalUsers; i++ ){
      if (users[i].id != "") {
        JsonObject& users_0 = usersNode.createNestedObject();
        users_0["ID"] = users[i].id;
        users_0["name"] = users[i].name;
        users_0["color"] = users[i].color;
        users_0["doze"] = int(users[i].doze);
        users_0["total"] = int(users[i].total);
        users_0["totalshots"] = int(users[i].totalShots);
      }
    }
    String out;
    root.printTo(out);
    HTTP.send(200, "application/json", out);
  });

  HTTP.on("/saveusers", HTTP_GET, []() {
    String jstr= HTTP.arg("json");
    DynamicJsonBuffer jsonBuffer;
  // Parse the JSON input
    JsonObject& jarr = jsonBuffer.parseObject(jstr);
  // Parse succeeded?
    JsonArray& jusers =jarr["users"];
    if (jusers.success()) {
  // Yes! We can extract values.
    //for (int i; i< jusers.size; i++) {
      int i = 0;
      for (JsonObject& user : jusers) { 
        //for (int i=0;i<totalUsers;i++){
          //String uid=user["ID"].as<String>();
          //if (users[i].id == String(uid)) {
            users[i].id = user["ID"].as<String>();
            users[i].name = user["name"].as<String>();
            users[i].doze = user["doze"];
            users[i].color = user["color"].as<String>();
            users[i].total = user["total"];
            users[i].totalShots = user["totalshots"];
            i++;
          //}
        //} 
      }
      while (i < 6) {
            users[i].id = "";
            users[i].name = "";
            users[i].doze = 0;
            users[i].color = "";
            users[i].total = 0;
            users[i].totalShots = 0;
            i++;
      }
      userCount();
      SocketData("userreload","1","99");
      HTTP.send(200, "text/plain", "OK");
    } else {
  // No!
  // The input may be invalid, or the JsonBuffer may be too small.
      HTTP.send(200, "text/plain", "Error");
    }
    
  });


  HTTP.on("/saveconfig", HTTP_GET, []() {
    String js= HTTP.arg("json");
    configSetup = js;
    saveConfig();
    FastLED.setBrightness(jsonReadtoInt(configSetup, "ledbright"));
    HTTP.send(200, "text/plain", "OK");
  });
  HTTP.on("/saveshot", HTTP_GET, []() {
    String js= HTTP.arg("json");
    //configSetup = js;
    //saveConfig();
    //FastLED.setBrightness(jsonReadtoInt(configSetup, "ledbright"));
    writeFile("shots.json", js );
    HTTP.send(200, "text/plain", "OK");
  });
  // HTTP.on("/users.json", HTTP_GET, []() {
  //   HTTP.send(200, "application/json", configSetup);
  // });
  // -------------------Выдаем данные configSetup
  HTTP.on("/restart", HTTP_GET, []() {
    String restart = HTTP.arg("device");          // Получаем значение device из запроса
    if (restart == "ok") {                         // Если значение равно Ок
      HTTP.send(200, "text / plain", "Reset OK"); // Oтправляем ответ Reset OK
      ESP.restart();                                // перезагружаем модуль
    }
    else {                                        // иначе
      HTTP.send(200, "text / plain", "No Reset"); // Oтправляем ответ No Reset
    }
  });
  // Добавляем функцию Update для перезаписи прошивки по WiFi при 1М(256K SPIFFS) и выше
  httpUpdater.setup(&HTTP);
  // Запускаем HTTP сервер
  HTTP.begin();
}

void SSDP_init(void) {
  // SSDP дескриптор
  HTTP.on("/description.xml", HTTP_GET, []() {
    SSDP.schema(HTTP.client());
  });
   // --------------------Получаем SSDP со страницы
  HTTP.on("/ssdp", HTTP_GET, []() {
    String ssdp = HTTP.arg("ssdp");
  configJson=jsonWrite(configJson, "SSDP", ssdp);
  configJson=jsonWrite(configSetup, "SSDP", ssdp);
  SSDP.setName(jsonRead(configSetup, "SSDP"));
  saveConfig();                 // Функция сохранения данных во Flash
  HTTP.send(200, "text/plain", "OK"); // отправляем ответ о выполнении
  });
  //Если версия  2.0.0 закаментируйте следующую строчку
  SSDP.setDeviceType("upnp:rootdevice");
  SSDP.setSchemaURL("description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName(jsonRead(configSetup, "SSDP"));
  SSDP.setSerialNumber("001788102201");
  SSDP.setURL("/");
  SSDP.setModelName("Cod-No-HTML");
  SSDP.setModelNumber("000000000001");
  SSDP.setModelURL("http://esp8266-arduinoide.ru/step9-codnohtml/");
  SSDP.setManufacturer("Tretyakov Sergey");
  SSDP.setManufacturerURL("http://www.esp8266-arduinoide.ru");
  SSDP.begin();
}

// ------------- Чтение значения json
String jsonRead(String &json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name].as<String>();
}

// ------------- Чтение значения json
int jsonReadtoInt(String &json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name];
}
// ------------- Чтение значения json bool
bool jsonReadtoBool(String &json, String name) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  return root[name].as<bool>();
}

// ------------- Запись значения json String
String jsonWrite(String &json, String name, String volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}

// ------------- Запись значения json int
String jsonWrite(String &json, String name, int volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}
// ------------- Запись значения json bool
String jsonWriteBool(String &json, String name, bool volume) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  root[name] = volume;
  json = "";
  root.printTo(json);
  return json;
}
// ------------- Поиск цвета по ID в shots.json
String findColor(String id){
  String jstr = readFile("shots.json", 2048);
  DynamicJsonBuffer jsonBuffer;
  // Parse the JSON input
  JsonObject& jshotsarr = jsonBuffer.parseObject(jstr);
  // Parse succeeded?
  JsonArray& jshots =jshotsarr["shots"];
  if (jshots.success()) {
  // Yes! We can extract values.
    //for (int i; i< jusers.size; i++) {
    for (JsonObject& shot : jshots) {  
      String shotID = shot["ID"].as<String>();
      if (shotID == id) return shot["color"].as<String>();
    }
  } else {
  // No!
  // The input may be invalid, or the JsonBuffer may be too small.
  return "Error";
  }
  return "grey";
}


void saveConfig (){
  writeFile("config.json", configSetup );
}

// ------------- Чтение файла в строку
String readFile(String fileName, size_t len ) {
  File configFile = SPIFFS.open("/" + fileName, "r");
  if (!configFile) {
    return "Failed";
  }
  size_t size = configFile.size();
  if (size > len) {
    configFile.close();
    return "Large";
  }
  String temp = configFile.readString();
  configFile.close();
  return temp;
}

// ------------- Запись строки в файл
String writeFile(String fileName, String strings ) {
  File configFile = SPIFFS.open("/" + fileName, "w");
  if (!configFile) {
    return "Failed to open config file";
  }
  configFile.print(strings);
  //strings.printTo(configFile);
  configFile.close();
  return "Write sucsses";
}

void WIFIinit() {
    /*
    // --------------------Получаем SSDP со страницы
    HTTP.on("/ssid", HTTP_GET, []() {
    jsonWrite(configSetup, "ssid", HTTP.arg("ssid"));
    jsonWrite(configSetup, "password", HTTP.arg("password"));
    saveConfig();                 // Функция сохранения данных во Flash
    HTTP.send(200, "text/plain", "OK"); // отправляем ответ о выполнении
    });
    // --------------------Получаем SSDP со страницы
    HTTP.on("/ssidap", HTTP_GET, []() {
    jsonWrite(configSetup, "ssidAP", HTTP.arg("ssidAP"));
    jsonWrite(configSetup, "passwordAP", HTTP.arg("passwordAP"));
    saveConfig();                 // Функция сохранения данных во Flash
    HTTP.send(200, "text/plain", "OK"); // отправляем ответ о выполнении
    });
  */
  if (!jsonReadtoBool(configSetup,"isAP")){
    // Попытка подключения к точке доступа
    WiFi.mode(WIFI_STA);
    byte tries = 11;
    String _ssid = jsonRead(configSetup, "wifiSSID");
    String _password = jsonRead(configSetup, "wifiPASS");
    curWifiPass=_password;
    curWifiSsid=_ssid;
    if (_ssid == "" && _password == "") {
      WiFi.begin();
    }
    else {
      WiFi.begin(_ssid.c_str(), _password.c_str());
    }
    // Делаем проверку подключения до тех пор пока счетчик tries
    // не станет равен нулю или не получим подключение
    while (--tries && WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(1000);
    }
    if (WiFi.status() != WL_CONNECTED)
    {
      // Если не удалось подключиться запускаем в режиме AP
      Serial.println("");
      Serial.println("WiFi up AP");
      StartAPMode();
    }
    else {
      // Иначе удалось подключиться отправляем сообщение
      // о подключении и выводим адрес IP
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }    
    Serial.print("SSID ");
    Serial.println(_ssid);
    Serial.print("PASS ");
    Serial.println(_password);
    jsonWrite(configSetup, "ip", WiFi.localIP().toString()); 
  } else {
    StartAPMode();
  }
}

bool StartAPMode() {
  IPAddress apIP(192, 168, 4, 1);
  // Отключаем WIFI
  WiFi.disconnect();
  // Меняем режим на режим точки доступа
  WiFi.mode(WIFI_AP);
  // Задаем настройки сети
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // Включаем WIFI в режиме точки доступа с именем и паролем
  // хронящихся в переменных _ssidAP _passwordAP
  String _ssidAP = jsonRead(configSetup, "APSSID");
  String _passwordAP = jsonRead(configSetup, "APPASS");
  curWifiSsid = _ssidAP;
  curWifiPass=_passwordAP;
  jsonWrite(configSetup, "ip", apIP.toString());
  WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
  return true;
}
//###############################################################
//   WEBSOCKET ##################################################
//###############################################################

void webSocket_init() {
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:  // Событие происходит при отключени клиента 
      Serial.println("web Socket disconnected");
      isWS=false;
      break;
    case WStype_CONNECTED: // Событие происходит при подключении клиента
      {
        Serial.println("web Socket Connected"); 
        isWS=true;
        //webSocket.sendTXT(num, configJson); // Отправим в всю строку с данными используя номер клиента он в num
      }
      break;
    case WStype_TEXT: // Событие происходит при получении данных текстового формата из webSocket
      // webSocket.sendTXT(num, "message here"); // Можно отправлять любое сообщение как строку по номеру соединения
      // webSocket.broadcastTXT("message here");
      break;
    case WStype_BIN:      // Событие происходит при получении бинарных данных из webSocket
      // webSocket.sendBIN(num, payload, length);
      break;
  }
}
// Отправка данных в Socket всем получателям
// Параметры Имя ключа, Данные, Предыдущее значение
void SocketData (String key, String data, String data_old)  {
  if (data_old != data && isWS) {
    String broadcast = "{}";
    jsonWrite(broadcast, key, data);
    webSocket.broadcastTXT(broadcast);
  }
}
void SocketSend (String broadcast)  {
  if (isWS){
    webSocket.broadcastTXT(broadcast); 
  }
}
void SocketSendDock(int pos) {
  if (isWS){
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonArray& dockNode = root.createNestedArray("dock");
    JsonObject& dock0 = dockNode.createNestedObject();
    dock0["pos"] = pos;
    dock0["state"] = dock[pos].state;
    dock0["user"] = dock[pos].user;
    if (dock[pos].user >= 0) dock0["id"] = users[dock[pos].user].id;
    String send;
    root.printTo(send);
    SocketSend(send);
  }
}