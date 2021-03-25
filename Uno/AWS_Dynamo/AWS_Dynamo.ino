////=============== Network libs ================//
//


#include "FS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h> //https://www.arduinolibraries.info/libraries/pub-sub-client
#include <NTPClient.h> //https://www.arduinolibraries.info/libraries/ntp-client
#include <WiFiUdp.h>

#include <SoftwareSerial.h>


#define BUFFER_LEN 256
char msg[BUFFER_LEN];
String currentDate;

//SoftwareSerial node_mcu(10,11); // (Tx,Rx)
const byte numChars = 200;
char receivedChars[numChars];   // an array to store the received data
String X(1000);
boolean newData = false;

const char* ssid = "IKEA"; //TP-Link_82DE
const char* password = "ArizonaIstToll!"; //67244737
//char data[1000];
char data[BUFFER_LEN];
String receivedString; 

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

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


// ============================================= CODE FOR STORING DATA ==========================================//
void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = 'x';
    char rc;
   
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc!=endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
          
            Serial.println("received end of line");
//            Serial.println(receivedChars[ndx-1]);
            Serial.println(ndx);
            receivedChars[ndx-1] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
//            data = receivedChars;
            delay(1000);

        }
    }
}


    // =================================== Code for fetcihng current date and time ============================//
void get_time(){
  Serial.println("inside time function");
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
  currentDate = "\"" + currentMonthString + "/" + monthDayString + "/" + String(currentYear)+ "-" + currentHourString + ":" + currentMinuteString + ":" + currentSecondString + "\"";

  String temp = receivedChars;
  receivedString = temp + ", \"Time\" : " + currentDate+ "}";
  

//  for (i=1;i<length(currentDate);i++)
//  {
//    receivedChars[sizeof(receivedChars)+i] = current
//    }

  
  }

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
//      client.subscribe("inTopic");
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
//    node_mcu.begin(115200);
//    pinMode(5,INPUT);
// Open serial communications and wait for port to open:
Serial.begin(115200);
setup_wifi();
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

  
//while (!Serial) {
//; // wait for serial port to connect. Needed for native USB port only
//}
}


void loop() { // run over and over
//  Serial.println("initial wait");
  delay(1000);
//  snprintf (msg, BUFFER_LEN, "{\"Time\" : \"%s\",\"Temperature\" : %f, \"Humidity\" : %f, \"UV_Intensity\" : %f, \"Dallas1\" : %f, \"Dallas2\" : %f, \"Wind_Speed\" : %f}","some time",10.0, 20.0, 30.0, 40.0, 50.0, 60.0);
//  Serial.println(msg);
  if (!client.connected()) {
    reconnect();
  }
  recvWithEndMarker();
    if(newData == true)
    {
      Serial.println(receivedString);
      get_time();
      receivedString.toCharArray(data, BUFFER_LEN);
      Serial.println("Publishing data to AWS");
      client.publish("outTopic", data);

      newData = false;

      
    }
    else
    {
      client.loop();
      Serial.println("Waiting for data");
        }


  
}

 







// ------------------------------------------------------------------------------------------- //

/*
#include <SoftwareSerial.h>

//SoftwareSerial node_mcu(10,11);
const byte numChars = 200;
char receivedChars[numChars];   // an array to store the received data
String X(1000);

boolean newData = false;

void setup() {
    Serial.begin(115200);
//    node_mcu.begin(115200);
//    pinMode(5,INPUT);
    Serial.println("<Arduino is ready>");
}


void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = 'x';
    char rc;
   
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
          Serial.println("end of the line");
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;

        }
    }
}

//void showNewData() {
//    if (newData == true) {
//        Serial.print("This just in ... ");
//        Serial.println(receivedChars);
//        newData = false;
//    }
//}

void loop() {
  Serial.println("waiting for data");
    recvWithEndMarker();
    if(newData == true){
//    showNewData();
  Serial.println(receivedChars);
  newData = false;
  delay(1000);}
    
}
*/
