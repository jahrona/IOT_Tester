#define WIFI_USE    //define this if using wifi before including Hendi-Internet library
#include <Hendi-Internet.h>

#include <ArduinoJson.h>

#define NODE_CODE "6a75180c-cbf8-47f5-9be8-43e1b7834917"


const int trigPin = 27;
const int echoPin = 22;
// defines variables
long duration;
int distance;

String wifi_name = "NOCOLA_DEV_ATAS";  
String wifi_pass = "NOCOLADEV2021";
unsigned long the_millis=0;
unsigned long current_time;

#define PIRpin 21
int sensorValue;  //variable to store sensor value

//max 6675, 5V
#define wSCK 14   //D14
#define wCS 15    //D15
#define wSO 12    //D12
#define wSI 13   //not used

#include <MAX6675_Thermocouple.h>
#include <SPI.h>                    
SPIClass* hspi = NULL;              // USE VIRTUAL SPI
Thermocouple* thermocouple;
float suhu;
float getCelcius() {
  return thermocouple->readCelsius();
}

void login_message(){     //not mandatory
  // Serial.println("Pura2nya publish saat pertama kali login mqtt");
  ptr_MQTT->publish("hendi/test-out/", "hehee");
  delay(100);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("WelCum");
  if(true){
    
  hspi = new SPIClass(HSPI);
  hspi->begin(wSCK, wSO, wSI, wCS);
  pinMode(wCS, OUTPUT);
  thermocouple = new MAX6675_Thermocouple(wSCK, wCS, wSO);
  }
  if(true){
    
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  }
  
  if(true){   // add in_topic
    addInTopic("hendi/test-in/");     //
  }
  
  while(WiFi.status() != WL_CONNECTED){   // connect wifi n free wifi
    connectWifi(wifi_name,wifi_pass);
  }
  
  // connect to mqtt
  connectMQTT(ptr_MQTT,"random_node_code",login_message); // ptr_MQTT default tidak berubah
  // connectMQTT(ptr_MQTT,"random_node_code");            // without login message
  // 
  // 
  // 
  // 
}

void loop() {
  // put your main code here, to run repeatedly:
  ptr_MQTT->loop();
  if(millis() - the_millis > 10000){
    digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
    current_time = getTimeWifi();
    the_millis = millis();
    suhu = getCelcius();
	  sensorValue = digitalRead(PIRpin); // read digital output pin
  }
  if(true){
    DynamicJsonDocument doc(100);
    String txt;
    
    doc["nodeCode"] = NODE_CODE;
    doc["time"] = (String)current_time + "000" ;
    doc["0"] = distance;
    doc["1"] = suhu;
    doc["2"] = sensorValue;

    serializeJsonPretty(doc, txt);
    ptr_MQTT->publish("flux/general_json/6a75180c-cbf8-47f5-9be8-43e1b7834917", txt.c_str());
      delay(1000);
  }
}
