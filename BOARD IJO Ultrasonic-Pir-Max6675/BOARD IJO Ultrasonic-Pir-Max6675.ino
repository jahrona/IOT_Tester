//max 6675, 5V
#define wSCK 14   //D14
#define wCS 15    //D15
#define wSO 12    //D12
#define wSI 13   //not used

#include <MAX6675_Thermocouple.h>
#include <SPI.h>                    
SPIClass* hspi = NULL;              // USE VIRTUAL SPI
Thermocouple* thermocouple;

#define PIRpin 21

int sensorValue;  //variable to store sensor value

const int trigPin = 27;
const int echoPin = 22;
// defines variables
long duration;
int distance;

float getCelcius() {
  return thermocouple->readCelsius();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  hspi = new SPIClass(HSPI);
  hspi->begin(wSCK, wSO, wSI, wCS);
  pinMode(wCS, OUTPUT);
  thermocouple = new MAX6675_Thermocouple(wSCK, wCS, wSO);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(PIRpin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.println(getCelcius());
  sensorValue = digitalRead(PIRpin); // read digital output pin
	Serial.print("Digital Output: ");
	Serial.print(sensorValue);
	
	// Determine the status
	if (sensorValue) {
		Serial.println("  Move Detected");
	} else {
		Serial.println("  No Motion");
	}
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
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);

  delay(5000);

}
