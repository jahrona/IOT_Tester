#define PRESSURE_PIN 4

#include <WiFi.h>
#include <ESP32Ping.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <stdio.h>

int r1 = 458;
int r2 = 458 + 458;
int r_tot = r1 + r2;

const char* mqttServer = "mqtt.nocola.co.id";
const int mqttPort = 1883;
const char* mqttUser = "mqtt_nocola";
const char* mqttPassword = "cinta123";
const char* node_code = "96e8dbea-fb7a-487b-b1da-f2a26ec89af3";

const char* outTopic = "flux/general_json/96e8dbea-fb7a-487b-b1da-f2a26ec89af3";
const char* inTopic = "flux/di/";

WiFiClient espClient; PubSubClient mqtt(espClient);
WiFiUDP ntpUDP; NTPClient timeClient(ntpUDP, "id.pool.ntp.org");

String ssid = "NOCOLA_DEV_ATAS";
String password = "NOCOLADEV2021";

float pressureValue() {
  //esp read 0.1V below as 0adc and 3.2v above as 4095

  //convert adc to voltage divider voltage
  float vol_div_limit = 1.00 * 4.5 * r2 / r_tot; //should below 3.2v
  
  int ana_val = analogRead(PRESSURE_PIN);
  Serial.print("Analog value is = "); Serial.println(ana_val);
  if(ana_val != 0){
  float vol_div_v = analogRead(PRESSURE_PIN) * vol_div_limit / 4095;
  float pressure_v = vol_div_v * r_tot / r2;

  //convert voltage to pressure value
  float volt_per_bar = 4 / 40;
  float pressure_val = (pressure_v - 0.5) / volt_per_bar;
  Serial.print("Pressure val is = "); Serial.println(pressure_val);
  return pressure_val;
  }
  else{
    return 0;
  }
}

bool connectWifi(){
  WiFi.disconnect(); delay(3000);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(),password.c_str());
  
  Serial.print("Connect to : "); Serial.println(ssid); delay(500);
  Serial.print("Pas: "); Serial.println(password); delay(500);
  Serial.println("Try connect wifi"); delay(200);

  int try_mark=0;
  while ((WiFi.status() != WL_CONNECTED) && (try_mark < 5)) {
    Serial.println("Try WiFi...");
    try_mark++;
    delay(1000);
  }
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Wifi Connected"); delay(100);
    if(Ping.ping("www.google.com",3)){    //ping google to check internet
      Serial.println("Connected to Internet");
      WiFi.setAutoReconnect(true);
      WiFi.persistent(true);
      return true;
    }
    else{
      Serial.println("No Internet..");
      return false;
    }
  }
  else{
    return false;
  }
  
}

bool connectMqtt(){
  if(!mqtt.connected()){
    String clientId = node_code;
//    if (mqtt.connect(clientId.c_str(), mqttUser, mqttPassword)){
    if (mqtt.connect(node_code, mqttUser, mqttPassword)){
//      mqtt.subscribe(relayTopic);
      Serial.println("MQTT Connected"); delay(100);
      char pubMsg[60]; sprintf(pubMsg,("%s%s"), node_code, " just logged in.");
      mqtt.publish(outTopic,(char*)pubMsg); delay(2000);  
      char msgMqtt[200];
//      Serial.println("Send data: "); delay(100);  Serial.println((char*)msgMqtt); delay(200);
//      mqtt.publish(outTopic, ((char*)msgMqtt)); delay(2000);
      return true;
    }
    else{
      Serial.print("failed, rc="); Serial.print(mqtt.state()); delay(200);
      return false;
    }
  }
  else{
    return false;
  }
}

bool freeWifi(){
  WiFi.disconnect();
  int wifi_available = WiFi.scanNetworks(); 
  delay(5000);  //scan network
  if(wifi_available==0){Serial.println("lol"); delay(100); return false;} //exit if none found
  else{
    unsigned int wifi_list[wifi_available];  // array of available free wifi
    int wifi_rssi_list[wifi_available]; //array of avalble free wifis rrsi
    unsigned int nopass_wifi=0; //total unpassworded wifi
    Serial.println(wifi_available); 
    for(int i=0; i<wifi_available; i++){  //filter free wifi from all wifi
      if(WiFi.encryptionType(i) == WIFI_AUTH_OPEN){
        wifi_list[nopass_wifi] = i;
        wifi_rssi_list[nopass_wifi] = WiFi.RSSI(i);
        nopass_wifi++;
      }
      delay(100);
    }
    Serial.println(nopass_wifi);
    if(nopass_wifi == 0){Serial.println("no free wifi");return false;}  //exit if no wifi found
    for(int i=0; i<nopass_wifi-1; i++){ // sort wifi by rssi
      for(int u=i+1; u<nopass_wifi; u++){
        if(wifi_rssi_list[u] > wifi_rssi_list[i]){
          int ii = wifi_rssi_list[i]; int uu = wifi_rssi_list[u];
          int iii = wifi_list[i]; int uuu = wifi_list[u];
          wifi_rssi_list[i] = uu; wifi_rssi_list[u] = ii;
          wifi_list[i] = uuu; wifi_list[u] = iii;
        }
      }
    }
    /*    SERIAL PRINTLN TO CHECK WIFI LIST
    for(int r=0; r<nopass_wifi; r++){
      Serial.println(wifi_list[r]);
      Serial.println(WiFi.SSID(wifi_list[r]));
      Serial.println(WiFi.RSSI(wifi_list[r]));
    }
    */
    for(int k=0; k<nopass_wifi; k++){   //try each free wifi, test the connection and exit if success
      String wifi_name = WiFi.SSID(wifi_list[k]);
      #if SERIAL_TEST == 1
      Serial.println("Connect to ");
      Serial.println(wifi_name);
      #endif
      WiFi.begin(WiFi.SSID(wifi_list[k]).c_str());
      int try_mark=0;
      while ((WiFi.status() != WL_CONNECTED) && (try_mark < 3)) {
        Serial.println("Try WiFi...");
        delay(1000);
        try_mark++;
      }
      if(Ping.ping("www.google.com",3)){    //ping google to check internet
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        Serial.println("Connected");
        ssid =  wifi_list[k];
        return true;
      }
    }
    return false;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Pressure Sensor");
  pinMode(PRESSURE_PIN, INPUT);
  if (true) {
    while(WiFi.status() != WL_CONNECTED) {
      if(connectWifi()){
        Serial.println("Connect to default wifi");
        delay(100);
        }
      else{
        freeWifi();
      }
    }
  }
  if (true) { //connect mqtt
    mqtt.setServer(mqttServer, mqttPort);
    //    mqtt.setCallback(callback);
    String clientId = "hendiihendiihendiihendiihendiihendii";
    if (mqtt.connect(clientId.c_str(), mqttUser, mqttPassword)) {
      mqtt.subscribe(inTopic);
      Serial.println("MQTT Connected"); delay(100);
      char pubMsg[60]; sprintf(pubMsg, ("%s"), " just logged in.");
      mqtt.publish(outTopic, ((char*)pubMsg)); delay(2000);
    }
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  DynamicJsonDocument doc(100);
  String txt;
  if(true) {
    doc["nodeCode"] = node_code;
    char time_str[20];
    sprintf(time_str,"%lu%s\0", timeClient.getEpochTime(),"000");
    doc["time"] = time_str;
//    char pressure_str[10];
//    dtostrf[pressureValue(), 1,2 pressure_str);
//    sprintf(pressure_str,"%s\0", pressure_str);
    doc["0"] = pressureValue() * 1.00;
    serializeJsonPretty(doc,txt);
  }
  if(mqtt.connected()){
      timeClient.forceUpdate();
      Serial.println("Send data: "); delay(100);
      Serial.println(txt); delay(200);
      mqtt.publish(outTopic, txt.c_str()); delay(1000);
      delay(1000);
    }
    else{
      Serial.print("failed mqtt, rc="); Serial.print(mqtt.state()); delay(200);
      connectMqtt();
    }
 delay(10000);
}
