#define TINY_GSM_MODEM_SIM800
// USING SOFTWARE SERIAL
#define gsmRX 32
#define gsmTX 33

#include <Arduino.h>
#include <PubSubClient.h>
#include <TinyGsmClient.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

SoftwareSerial gprsSerial(gsmTX,gsmRX);
TinyGsm modem(gprsSerial);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

//APN 3
// const char apn[]  = "APN TRI";
// const char user[] = "3gprs";
// const char pass[] = "3gprs";
const char apn[]  = "3 data";
const char user[] = "";
const char pass[] = "";

//APN XL
// const char apn[]  = "xlunlimited";
// const char user[] = "";
// const char pass[] = "";

//mqtt
const char* mqttServer = "mqtt.nocola.co.id";
const int mqttPort = 1883;
const char* mqttUser = "mqtt_nocola";
const char* mqttPassword = "cinta123";

const char* outTopic = "hendi/";


void showSerialData(){
  while(gprsSerial.available()!=0){
  Serial.write(gprsSerial.read());
  // delay(1000); 
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
  String clientId = "GPRS hendi";
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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("Hellow :)"));
  gprsSerial.begin(9600);
  delay(100);
  atInput("AT","Awake ");
  atInput("AT+CSCLK=0", "Turn on... ");
  reconnectGPRS();
  mqtt.setServer(mqttServer, mqttPort);
  connectMqtt();
}

void loop() {
  mqtt.publish(outTopic, "hendi\0");
  delay(5000);
}
