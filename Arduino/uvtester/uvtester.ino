//D0 1st Dallas Sensor
//D1 2nd Dallas Sensor
//D3 3rd Dallas Sensor
//D4 DHT Sensor

//A0 UV Sensor

#include <OneWire.h> 
#include <DallasTemperature.h>
#include "DHT.h"

#define DHTTYPE DHT22
#define DHTPIN  0
#define Dallas_1 5

//float Dallas_2= 5;
//float Dallas_3= 4;
int UVOUT = A0;


OneWire oneWire(Dallas_1); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/ 

DHT dht(DHTPIN, DHTTYPE);



void setup() {
  Serial.begin(9600);
  pinMode(UVOUT, INPUT);
  dht.begin();

}

void loop()
{
  int uvLevel = averageAnalogRead(UVOUT);
  float analog = analogRead(A0);
//  int refLevel = averageAnalogRead(REF_3V3);
  
  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  float outputVoltage = 3.3*uvLevel/1024;
  
  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0); //Convert the voltage to a UV intensity level
 
//  Serial.print("output: ");
//  Serial.print(refLevel);

  Serial.print("Analog output");
  Serial.print(analog);
  
  Serial.print("ML8511 output: ");
  Serial.print(uvLevel);
 
  Serial.print(" / ML8511 voltage: ");
  Serial.print(outputVoltage);
 
  Serial.print(" / UV Intensity (mW/cm^2): ");
  Serial.print(uvIntensity);
  
  Serial.println();

  Serial.print("Temperature 1 is: "); 
  Serial.print(sensors.getTempCByIndex(0));
  Serial.println();

//  Serial.print("Temperature 2 is: "); 
//  Serial.print(sensors.getTempCByIndex(1));
//  Serial.println();
//
//  Serial.print("Temperature 3 is: "); 
//  Serial.print(sensors.getTempCByIndex(2));
//  Serial.println();

  Serial.print("DHT values:");
  float h = dht.readTemperature();
  float f = dht.readHumidity();
  Serial.print(h);
  Serial.print(f);
  Serial.println();
  
  delay(2000);
}
 
//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 
 
  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
 
  return(runningValue);
}
 
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
