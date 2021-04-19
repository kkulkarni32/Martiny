//========== PIN CONFIGURATION =================//
//UV sensor (A0 & A1)
//DSB120 Dallas sensor D1 (pin4)
//DHT22 D3 (pin3)
//Wind sensor (A2)


#include "DHT.h"                // DHT Sensor
#include <OneWire.h>            // For Dallas sensor communication
#include <DallasTemperature.h>  // For Dallas sensor reading
#include <SoftwareSerial.h>
SoftwareSerial espSerial(10,11); //Tx, Rx


#define UVOUT A0   
#define DHTPIN 3   
#define DHTTYPE DHT22
#define ONE_WIRE_BUS 4
#define BUFFER_LEN 256

char msg[BUFFER_LEN];
float end_time = 0;
int REF_3V3 = A1;



//====================== Dallas ====================//
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

//====================== DHT ==================================//
DHT dht(DHTPIN, DHTTYPE);

//====================== Wind Sensor =========================//
int windSensor = A4;
float voltageMax = 2.0;
float voltageMin = .454; // Make sure the value is 3 decimal points, serial print by default displays only 2 decimal points. Currently the offset voltage is .405, but to be safer I havemade .409. This can vary for every sensor, make sure to check.
float voltageConversionConstant = .004882814;
float sensorVoltage = 0;
float windSpeed = 0;

float windSpeedMin = 0;
float windSpeedMax = 32;

void setup() {
  Serial.begin(9600);
  espSerial.begin(115200);
  //========= UV sensor ===========//
  pinMode(UVOUT, INPUT);
  pinMode(A2, INPUT);

  //========== Dallas ============//
  sensors.begin();
  
  //=========== DHT ==============//
  dht.begin();

  //=========== Wind sensor =========//


}

void loop() {
  Serial.println("hahahaha");
  float start = millis();
  float uvIntensity = 0;
  float dallas_0 = 0;
  float dallas_1 = 0;
  float temperature = 0;
  float humidity = 0;
  float windSpeed = 0;

  float start_time =millis();
  int average = 0;

//  for (int x=0; x<=average; x++){
  while (end_time-start_time<60000){
  
  delay(1000);

  //========================== UV sensor ==============================//
//  int uvLevel = averageAnalogRead(UVOUT);
  int uvLevel = analogRead(UVOUT);
//  Serial.println("UV voltage");
//  Serial.println(uvLevel);
  int refLevel = analogRead(REF_3V3);
//  float analog = analogRead(A0);

//  Serial.println("uvlevel ");
//  Serial.println(uvLevel);
//
//  Serial.println("reflevel");
//  Serial.println(refLevel);
//  
  float outputVoltage = 3.3/refLevel *uvLevel;
//  Serial.println("UV voltage");
//  Serial.println(outputVoltage);

  if (outputVoltage<1) // If lesser than the minimum
  {
    uvIntensity = 0; 
    }

   else
   {
      uvIntensity += mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); //Convert the voltage to a UV intensity level
   }

  //=========================== Dallas ===============================//
  sensors.requestTemperatures();
  dallas_0 += sensors.getTempCByIndex(0);
  dallas_1 += sensors.getTempCByIndex(1);
  
  //=========================== DHT ==================================//
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(h) || isnan(t)) {
//    Serial.println(F("Failed to read from DHT sensor!"));
    bool DHT_flag = true;
  }

  else{
    temperature += t;
    humidity += h;
  }

  //======================== Wind sensor ============================//
  int sensorValue = analogRead(windSensor);
  delay(20);

  sensorValue = analogRead(windSensor);

  float voltage = sensorValue * (5.0 / 1023.0);

  sensorVoltage = sensorValue * voltageConversionConstant;
  Serial.println();
  
  Serial.println("Raw values");
  Serial.println(sensorValue);

  Serial.println();
  
  Serial.println("Wind Sensor voltage");
  Serial.println(voltage, 3);
  
  if (sensorVoltage > voltageMin and sensorVoltage <= voltageMax) {
    Serial.println("HERE");
    windSpeed += ((sensorVoltage - voltageMin) * windSpeedMax / (voltageMax - voltageMin)); //For voltages above minimum value, use the linear relationship to calculate wind speed. * 2.232694
  }
//  Serial.println(windSpeed);

  average++;

  end_time=millis();
  
  }
  
//  Serial.println(average);
  Serial.print("Speed:");
  Serial.println(windSpeed/average);
  
  Serial.println();

  Serial.print("Temperature Dallas 0: "); 
  Serial.print(dallas_0/average);
  Serial.println();

  Serial.print("Temperature Dallas 1: "); 
  Serial.print(dallas_1/average);
  Serial.println();

  Serial.print("UV Intensity (mW/cm^2): ");
  Serial.print(uvIntensity/average);
  
  Serial.println();

  float end = millis();
  Serial.println("time difference");
  Serial.println(end-start);

//================================================== CODE FOR AWS =================================================//

  String msg2 = "{\"Temperature\"   : " + String(temperature/average) + ",\"Humidity\"  : " + String(humidity/average) + ",\"UV_Intensity\" : " + String(uvIntensity/average) + ",\"Dallas1\" : " + 
  String(dallas_0/average) + ",\"Dallas2\" : " + String(dallas_1/average) + ",\"Wind_Speed\" : " + String(windSpeed/average) + "}x";
  
  Serial.println("Sending data to node mcu");
  Serial.println(msg2);
  espSerial.println(msg2);

//  msg= msg2;
//  espSerial.write("I am writing some data here x");

}

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
