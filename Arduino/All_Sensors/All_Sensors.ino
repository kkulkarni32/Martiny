//========== PIN CONFIGURATION =================//
//UV sensor A0
//DSB120 Dallas sensor D1 (GPIo5)
//DHT22 D3 (GPIO0)
//Wind sensor D4 (GPIO2)


#include "DHT.h"                // DHT Sensor
#include <OneWire.h>            // For Dallas sensor communication
#include <DallasTemperature.h>  // For Dallas sensor reading

//int UVOUT = A0;
#define UVOUT A0   
#define DHTPIN 0   
#define DHTTYPE DHT22
#define ONE_WIRE_BUS 5

int average = 35; // Averaing over one minute

//====================== Wind Sensor ==========================//

const byte interruptPin = 2;
//volatile byte state = LOW;
int voltage = 0;
//float wind = 0;
volatile int prev_count = 0;
volatile int current_count = 0;

//====================== Dallas ====================//
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

//====================== DHT ==================================//
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  //========= UV sensor ===========//
  pinMode(UVOUT, INPUT);

  //========== Dallas ============//
  sensors.begin();
  
  //=========== DHT ==============//
  dht.begin();

  //=========== Wind sensor =========//
//  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), count_speed, RISING);
}

void loop() {
  float start = millis();
  float uvIntensity = 0;
  float dallas_0 = 0;
  float dallas_1 = 0;
  float temperature = 0;
  float humidity = 0;
  float wind_speed = 0;

  for (int x=0; x<=average; x++){
  
  delay(1000);

  //========================== UV sensor ==============================//
//  int uvLevel = averageAnalogRead(UVOUT);
  float uvLevel = analogRead(UVOUT);
//  float analog = analogRead(A0);
  float outputVoltage = 3.3*uvLevel/1024;
  uvIntensity += mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0); //Convert the voltage to a UV intensity level

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
  wind_speed += calc_speed();
//  int end = millis();

  if (current_count>1000) {current_count =0; prev_count=0;}; // resetting the count to avoid overflow
  }
  
//  voltage = digitalRead(interruptPin);
//  digitalWrite(LED_BUILTIN, state);
  Serial.print("Count:");
  Serial.println(current_count);

  Serial.print("Speed:");
  Serial.println(wind_speed/average);
  
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

// Important: Use the keword ICACHE_RAM_ATTR to make a function as a isr
ICACHE_RAM_ATTR void count_speed() { 
//  state = !state;
  current_count++;
}

float calc_speed(){
//  Serial.println(millis());
  float wind = (current_count - prev_count)*0.03768;
  
  prev_count = current_count;

  return wind;
}
