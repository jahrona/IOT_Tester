//wake, take gps, connect mqtt, send lat, long

#define TINY_GSM_MODEM_SIM800
#define donePin 6
#define nodeCode 18
#define gpsRX 5
#define gpsTX 4
#define numberOfSignal 10
#define the8loop 1
#define the2loop 1

#include <Arduino.h>
#include <PubSubClient.h>
#include <TinyGsmClient.h>
#include <TinyGPSPlus.h>
#include <LowPower.h>
#include <timestamp32bits.h>

IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);


#include <SoftwareSerial.h>
SoftwareSerial gprsSerial(3,2);
TinyGPSPlus gps;
SoftwareSerial ss(gpsTX, gpsRX);
unsigned long gpsTimeOut = 240000;

TinyGsm modem(gprsSerial);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

//time
timestamp32bits stamp = timestamp32bits();
unsigned long gpsTimeStamp;
uint16_t gpsYear;
uint8_t gpsMonth, gpsDay, gpHour, gpsMinute, gpsSecond;
float gpsLat,gpsLong;

//APN telkomsel
//const char apn[]  = "internet";
//const char user[] = "";
//const char pass[] = "";

//APN XL
//const char apn[]  = "xlunlimited";
//const char user[] = "";
//const char pass[] = "";

//APN AXIS
const char apn[]  = "AXIS";
const char user[] = "AXIS";
const char pass[] = "123456";

//mqtt
const char* mqttServer = "mqtt.nocola.co.id";
const int mqttPort = 1883;
const char* mqttUser = "mqtt_nocola";
const char* mqttPassword = "cinta123";

//const char* node_code = "7f853d06-7d13-47b7-82aa-c1b3333c6768"; //1
//const char* node_code = "ced2125a-0175-49db-8e11-139f1cb9b28e"; //2
const char* node_code = "cf3fbb99-8208-48e6-bbb7-71700a91cb67"; //3

//const char* outTopic = sprintf(outTopic,"%s%s","flux/xhib/",node_code);
//const char* outTopic = "flux/xhib/7f853d06-7d13-47b7-82aa-c1b3333c6768";
//const char* outTopic = "flux/xhib/ced2125a-0175-49db-8e11-139f1cb9b28e";
const char* outTopic = "flux/xhib/cf3fbb99-8208-48e6-bbb7-71700a91cb67";

char messages[100];
int16_t currentSignal;
int reset_counter=0;

void showSerialData();
void showSerialDataVar();
void atInput(String atCommand, String description);
bool connectMqtt();
void sim800lSetup();
void reconnectGPRS();
bool getGPS(unsigned long startTime);
unsigned long getGPSmillis(uint16_t theYear, uint8_t theMonth, uint8_t theDay, uint8_t theHour,  uint8_t theMinute,  uint8_t theSecond);
void tplPwrDwn();
void(*mulai_reset) (void) = 0; 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("Hellow :)"));
  delay(50);
  gprsSerial.begin(9600);
  ss.begin(9600);
  pinMode(donePin,OUTPUT);
//  atInput("AT", "Check Interface");
//  Serial.println("Modem: " + modem.getModemInfo());
//  delay(5000);
  mqtt.setServer(mqttServer, mqttPort);
//  reconnectGPRS();
//  atInput("AT","Awake ");
//  atInput("AT+CSCLK=0", "Turn on... ");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  ss.listen();
  bool gpsFlag= false;
  unsigned long gpsStart = millis();
  gpsFlag = getGPS(gpsStart);
  if(gpsFlag == true){ //command if success get gps
//    Serial.print(gps.location.lat(), 15);
//    Serial.print(F(","));
//    Serial.println(gps.location.lng(), 15);
    Serial.println(gps.speed.kmph(), 2);
    Serial.println(gps.altitude.meters(), 3);
    delay(100);
    gprsSerial.listen();
    atInput("AT","Awake ");
    atInput("AT+CSCLK=0", "Turn on... ");
    if(!mqtt.connected()){    
      reconnectGPRS();
      if(connectMqtt() == false){
        reset_counter+=1;      
      }
      else{
        reset_counter=0;
      }
      if(reset_counter >=3){
        Serial.println(F("restart"));
        delay(1000);
        mulai_reset();
      }
    }
    if(mqtt.connected()){
      //command if internet connect
      gpsTimeStamp= getGPSmillis((gps.date.year()%2000),gps.date.month(),gps.date.day(),gps.time.hour(),gps.time.minute(),gps.time.second());
      char gpsLatArray[20];
      char gpsLngArray[20];
      char gpsAltitudeArray[7];
      char gpsSpeedArray[5];

      dtostrf(gps.location.lat(),1,8,gpsLatArray);
      dtostrf(gps.location.lng(),1,8,gpsLngArray);
      dtostrf(gps.speed.kmph(), 1, 1, gpsSpeedArray);
      dtostrf(gps.altitude.meters(), 1, 1, gpsAltitudeArray);
      char msg[110];
//      sprintf(gpsAltitudeArray,"%s\0",gpsAltitudeArray);
//      sprintf(msg,"%s;%lu%s;%s;%s\0",node_code,gpsTimeStamp,"000",gpsLatArray,gpsLngArray);
//      mqtt.publish(outTopic, msg);
//      delay(5000);
//      sprintf(msg,"%s;%lu%s;%s;%s;%f\0",node_code,gpsTimeStamp,"000",gpsLatArray,gpsLngArray,gps.speed.kmph());
//      sprintf(msg,"%s;%s;%s\0",msg,gpsSpeedArray,gpsAltitudeArray);
      sprintf(msg,"%s;%lu%s;%s;%s;%s;%s\0",node_code,gpsTimeStamp,"000",gpsLatArray,gpsLngArray,gpsAltitudeArray,gpsSpeedArray);
      Serial.println((char*)msg);
      mqtt.publish(outTopic, msg);
      delay(5000);
    }
    else{
      //command if internet not connect
    }
    atInput("AT+CSCLK=2", "Go to sleep... ");
  }
  else{ //command if fail get gps
    Serial.println(F("NUOOOO"));
    delay(50);
    gprsSerial.listen();
    atInput("AT","Awake ");
    atInput("AT+CSCLK=0", "Turn on... ");
    reconnectGPRS();
    connectMqtt();
    if(mqtt.connected()){
      mqtt.publish(outTopic, "Hmm sadddd -HENDI");
    }
  }
//  tplPwrDwn();
  delay(20000);
}
//
//void tplPwrDwn(){
//  for(byte i= 0; i<numberOfSignal; i++){
//    delay(100);
//    digitalWrite(donePin,HIGH);
//    delay(100);
//    digitalWrite(donePin, LOW);
//  }
//}

unsigned long getGPSmillis(uint16_t theYear, uint8_t theMonth, uint8_t theDay, uint8_t theHour,  uint8_t theMinute,  uint8_t theSecond){
  unsigned long currentTimeStamp = stamp.timestamp(theYear, theMonth, theDay, theHour, theMinute, theSecond);
  return currentTimeStamp;
}

bool getGPS(unsigned long startTime){
  bool locFlag = false;
  bool dateFlag = false;
  bool timeFlag = false;
  bool gpsFlag = false;
  while((millis() - startTime < gpsTimeOut) && ((locFlag == false) || (dateFlag == false) || (timeFlag == false))){
    while(ss.available()>0){
      if(gps.encode(ss.read())){
        if ((gps.location.isValid())&& (locFlag==false)){
          locFlag = true;
          Serial.println("loc");
          delay(50);
        }
        if((gps.date.isValid()) && (dateFlag==false)){
          dateFlag = true;
          Serial.println("flag");
          delay(50);
        }
        if((gps.time.isValid()) && (timeFlag==false)){
          timeFlag = true;
          Serial.println("time");
          delay(50);
        }
      }
    }
  }
  if((locFlag == true) && (dateFlag == true) && (timeFlag == true)){
    gpsFlag = true;
    Serial.println(F("GG"));
    delay(5);
  }
  if(gps.charsProcessed() < 10){
    Serial.println(F("WW"));
    delay(5);
  }
  Serial.print("Chars processed :");
  Serial.println(gps.charsProcessed());
  return gpsFlag;
}

void showSerialData(){
  while(gprsSerial.available()!=0){
  Serial.write(gprsSerial.read());
  // delay(1000); 
  }
}

void showSerialDataVar(){
  while(gprsSerial.available()!=0){
  char c = gprsSerial.read();
  sprintf(messages,"%s%c",messages,c);
  }
}

void atInput(String atCommand, String description){
  Serial.print(description);
  gprsSerial.println(atCommand);
  delay(2000); 
  showSerialData();
}

void reconnectGPRS(){
    Serial.println("Reconnect GPRS");
    delay(50);
    if(!modem.waitForNetwork(10000L)){
      Serial.println("No network");
      delay(50);
    }
    Serial.println("Signal Quality: " + String(modem.getSignalQuality()));
    Serial.println("Connected to GPRS: " + String(apn));
    if (!modem.gprsConnect(apn, user, pass)){
      Serial.println("fail reconnect gprs");
      delay(50);
    }
    else{
      Serial.println("success");
      delay(50);
    }
}

bool connectMqtt(){
  if(!mqtt.connected()){
  Serial.print("Attempting MQTT connection...");
  String clientId = "GPRS Mobil";
  clientId += String(random(0xffff), HEX);
  if (mqtt.connect(clientId.c_str(), mqttUser, mqttPassword)){
    Serial.println(F("connected"));
    delay(20);
    return true;
  }
  else {
    Serial.print(F("failed, rc="));
    Serial.print(mqtt.state());
    return false;
    }
  }
  else{
    return true;
  }
}
