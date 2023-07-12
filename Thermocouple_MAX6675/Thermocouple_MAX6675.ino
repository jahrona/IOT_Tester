//max 6675, 5V
#define wSCK 14   //D14
#define wCS 15    //D15
#define wSO 12    //D12
#define wSI 13   //not used

#include <MAX6675_Thermocouple.h>
#include <SPI.h>                    
SPIClass* hspi = NULL;              // USE VIRTUAL SPI
Thermocouple* thermocouple;

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
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(getCelcius());
  delay(5000);

}
