//========== PIN CONFIGURATION =================//
//UV sensor A0
//DSB120 Dallas sensor D1 (GPIo5)
//DHT22 D3 (GPIO0)
//Wind sensor D4 (GPIO2)

//=============== Network libs ================//

#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h> //https://www.arduinolibraries.info/libraries/pub-sub-client
#include <NTPClient.h> //https://www.arduinolibraries.info/libraries/ntp-client
#include <WiFiUdp.h>

//============= Sensor libs ====================//
#include "DHT.h"                // DHT Sensor
#include <OneWire.h>            // For Dallas sensor communication
#include <DallasTemperature.h>  // For Dallas sensor reading


#define UVOUT A0   
#define DHTPIN 0   
#define DHTTYPE DHT22
#define ONE_WIRE_BUS 5


int average = 60; // Averaging over one minute
float end_time = 0;

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

//====================== Wind Sensor ==========================//

const byte interruptPin = 2;
//volatile byte state = LOW;
int voltage = 0;
//float wind = 0;
volatile int prev_count = 0;
volatile int current_count = 0;

//====================== Dallas ============================//
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

//====================== DHT ==================================//
DHT dht(DHTPIN, DHTTYPE);

// Update these with values suitable for your network.

const char* ssid = "IKEA";
const char* password = "ArizonaIstToll!";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

const char* AWS_endpoint = "a3ik45ic228w7y-ats.iot.us-east-2.amazonaws.com"; //MQTT broker ip

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
}


WiFiClientSecure espClient;
PubSubClient client(AWS_endpoint, 8883, callback, espClient); //set MQTT port number to 8883 as per //standard
//client.setKeepAlive(60);
//============================================================================
#define BUFFER_LEN 256
long lastMsg = 0;
char msg[BUFFER_LEN];
int value = 0;
byte mac[6];
char mac_Id[18];
//============================================================================

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  espClient.setBufferSizes(512, 512);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  timeClient.begin();
  timeClient.setTimeOffset(-25200);
  while(!timeClient.update()){
    timeClient.forceUpdate();
  }
  
  espClient.setX509Time(timeClient.getEpochTime());

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
  
  // Attempt to connect
    if (client.connect("ESPthing")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } 
    else {
    Serial.print("failed, rc=");
    Serial.print(client.state());
    Serial.println(" try again in 5 seconds");
    
    char buf[256];
    espClient.getLastSSLError(buf,256);
    Serial.print("WiFiClientSecure SSL error: ");
    Serial.println(buf);
    
    // Wait 5 seconds before retrying
    delay(5000);
    }
  }
}

void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  setup_wifi();
  delay(1000);
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
    return;
  }
  
  Serial.print("Heap: "); 
  Serial.println(ESP.getFreeHeap());
  
  // Load certificate file
  File cert = SPIFFS.open("/cert.der", "r"); //replace cert.crt eith your uploaded file name
  if (!cert) {
    Serial.println("Failed to open cert file");
  }
  
  else {
  Serial.println("Success to open cert file");
  }
  
  delay(1000);
  
  if (espClient.loadCertificate(cert))
    Serial.println("cert loaded");
  else
    Serial.println("cert not loaded");
  
  // Load private key file
  File private_key = SPIFFS.open("/private.der", "r"); //replace private eith your uploaded file name
  if (!private_key) {
    Serial.println("Failed to open private cert file");
  }
  else
    Serial.println("Success to open private cert file");
  
  delay(1000);
  
  if (espClient.loadPrivateKey(private_key))
    Serial.println("private key loaded");
  else
    Serial.println("private key not loaded");
  
  // Load CA file
  File ca = SPIFFS.open("/ca.der", "r"); //replace ca eith your uploaded file name
  if (!ca) {
    Serial.println("Failed to open ca ");
  }
  else
    Serial.println("Success to open ca");
  
  delay(1000);
  
  if(espClient.loadCACert(ca))
    Serial.println("ca loaded");
  else
    Serial.println("ca failed");
  
  Serial.print("Heap: "); 
  Serial.println(ESP.getFreeHeap());
  
  //===========================================================================
  WiFi.macAddress(mac);
  snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x",
  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(mac_Id);
  
  //============================ Sensor setup ================================================
  
  //========= UV sensor ===========//
  pinMode(UVOUT, INPUT);
  
  //========== Dallas ============//
  sensors.begin();
  
  //=========== DHT ==============//
  dht.begin();
  
  //=========== Wind sensor =========//
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), count_speed, RISING);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //
  //long now = millis();
  //if (now - lastMsg > 2000) {
  //lastMsg = now;
  //============================================================================
  
  
  // ==================================== Reading sensor data =========================//
  
  float start_time = millis();
//  Serial.println("start time.................................................");
//  Serial.print(start_time);
  int x = 0;
  float uvIntensity = 0;
  float dallas_0 = 0;
  float dallas_1 = 0;
  float temperature = 0;
  float humidity = 0;
  float wind_speed = 0;
  
//  for (int x=0; x<=average; x++){
    while (end_time-start_time<60000){

//      Serial.println("inside while looooop......................................................");
      client.loop();

//      if(x%5==0){
//        client.loop();
//    }
  
//    delay(1000);
    
    //========================== UV sensor ==============================//
    //  int uvLevel = averageAnalogRead(UVOUT);
    float uvLevel = analogRead(UVOUT);
//    Serial.println("ANALOG VOLTAGE");
//    Serial.println(uvLevel);
    //  float analog = analogRead(A0);
    float outputVoltage = 3.3*uvLevel/1024;
    uvIntensity += mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0); //Convert the voltage to a UV intensity level
    //Serial.print("Current UV: "); Serial.print(mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0)); Serial.println();
    
    //=========================== Dallas ===============================//
    sensors.requestTemperatures();
    dallas_0 += sensors.getTempCByIndex(0);
    dallas_1 += sensors.getTempCByIndex(1);
    //Serial.print("Current t air: "); Serial.print(sensors.getTempCByIndex(0)); Serial.println();
    //Serial.print("Current t globe: "); Serial.print(sensors.getTempCByIndex(1)); Serial.println();
    
    //=========================== DHT ==================================//
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    if (isnan(h) || isnan(t)) {
      bool DHT_flag = true;
    }
    
    else{
      //Serial.print("Current DHT t: "); Serial.print(t); Serial.println();
      //Serial.print("Current DHT h: "); Serial.print(h); Serial.println();
      temperature += t;
      humidity += h;
    }
    
    //======================== Wind sensor ============================//
    wind_speed += calc_speed();
        
    if (current_count>1000) {
      current_count =0; prev_count=0;
    } // resetting the count to avoid overflow

    end_time=millis();
    x++;
  }
  
  //  voltage = digitalRead(interruptPin);
  //  digitalWrite(LED_BUILTIN, state);
  Serial.println("Value of count");
  Serial.print(x);
  Serial.println();
  
  Serial.print("Count:");
  Serial.println(current_count);
  
  Serial.print("Speed:");
  Serial.println(wind_speed/60);
  
  Serial.print("Temperature Dallas 0: "); 
  Serial.print(dallas_0/x);
  Serial.println();
  
  Serial.print("Temperature Dallas 1: "); 
  Serial.print(dallas_1/x);
  Serial.println();
  
  Serial.print("UV Intensity (mW/cm^2): ");
  Serial.print(uvIntensity/x);
  
  Serial.println();
  
  float end = millis();
  Serial.println("time difference");
  Serial.println(end-start_time);
  
  
  // =================================== Code for fetcihng current date and time ============================//
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  
  int currentHour = timeClient.getHours();
  String currentHourString = String(currentHour);
  if (currentHourString.length()==1) {
    currentHourString = "0" + currentHourString;
  }  
  
  int currentMinute = timeClient.getMinutes();
  String currentMinuteString = String(currentMinute);
  if (currentMinuteString.length()==1) {
    currentMinuteString = "0" + currentMinuteString;
  } 

   
  int currentSecond = timeClient.getSeconds();
  String currentSecondString = String(currentSecond);
  if (currentSecondString.length()==1) {
    currentSecondString = "0" + currentSecondString;
  } 
  
  String weekDay = weekDays[timeClient.getDay()];  
  
  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  
  int monthDay = ptm->tm_mday;
  String monthDayString = String(monthDay);
  if (monthDayString.length()==1) {
    monthDayString = "0" + monthDayString;
  }
   
  int currentMonth = ptm->tm_mon+1;
  String currentMonthString = String(currentMonth);
  if (currentMonthString.length()==1) {
    currentMonthString = "0" + currentMonthString;
  }
    
  
  String currentMonthName = months[currentMonth-1];
  
  int currentYear = ptm->tm_year+1900;
  
  //Print complete date:
  String currentDate = currentMonthString + "/" + monthDayString + "/" + String(currentYear)+ "-" + currentHourString + ":" + currentMinuteString + ":" + currentSecondString;

  String macIdStr = mac_Id;
  
  snprintf (msg, BUFFER_LEN, "{\"Time\" : \"%s\",\"Temperature\" : %f, \"Humidity\" : %f, \"UV_Intensity\" : %f, \"Dallas1\" : %f, \"Dallas2\" : %f, \"Wind_Speed\" : %f}",currentDate.c_str(),temperature/x, humidity/x, uvIntensity/x, dallas_0/x, dallas_1/x, wind_speed/60);
//  snprintf (msg, 75, "{\"Temperature\": %f, \"Humidity\": %f, \"time\": \"%s\"}", temperature/x, humidity/x, currentDate.c_str());
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish("outTopic", msg);
  float e2 = millis();

  Serial.println("second time difference");
  Serial.print(e2-start_time);
  //=============================================================================
  Serial.print("Heap: "); Serial.println(ESP.getFreeHeap()); //Low heap can cause problems
  //}
  delay(1000); // wait for a second
}

 
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

ICACHE_RAM_ATTR void count_speed() { 
  current_count++;
}

float calc_speed(){
  float wind = (current_count - prev_count)*0.03768;
  
  prev_count = current_count;

  return wind;
}
