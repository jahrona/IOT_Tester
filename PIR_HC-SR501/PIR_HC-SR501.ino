#define PIRpin 21

int sensorValue;  //variable to store sensor value

void setup() {
	Serial.begin(9600); // sets the serial port to 9600
  Serial.println("PIR");
}

void loop() {
	sensorValue = digitalRead(PIRpin); // read digital output pin
	Serial.print("Digital Output: ");
	Serial.print(sensorValue);
	
	// Determine the status
	if (sensorValue) {
		Serial.println("  Move Detected");
	} else {
		Serial.println("  No Motion");
	}
	
	delay(2000); // wait 2s for next reading
}