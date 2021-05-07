#include "WiFiEsp.h"

#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#include "GravityTDS.h"
#define TdsSensorPinA A1
GravityTDS gravityTdsA;

float temperature = 25,tdsValue = 0;

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

int jam = 0;
int state = 0;

int relay1 = 8;
int relay2 = 9;
int relay3 = 10;
int relay4 = 11;
int relay5 = 12;
int relay6 = 13;

String datatemp = "";
String datahum = "";
String datatds = "";
String dataL1 = "";
String dataL2 = "";
String dataL3 = "";
String dataL4 = "";
String dataL5 = "";
String dataL6 = "";

int ldr1 = A8;
int ldr2 = A9;
int ldr3 = A10;
int ldr4 = A11;
int ldr5 = A12;
int ldr6 = A13;

int valueldr1 = 0;
int valueldr2 = 0;
int valueldr3 = 0;
int valueldr4 = 0;
int valueldr5 = 0;
int valueldr6 = 0;


#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial3(14, 15); // RX, TX
#endif

char ssid[] = "ainazman";            // your network SSID (name)
char pass[] = "87654321";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char server[] = "api.thingspeak.com";

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
//const unsigned long postingInterval = 10000L;

// Initialize the Ethernet client object
WiFiEspClient client;

void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);
  dht.begin();
  
  gravityTdsA.setPin(TdsSensorPinA);
  gravityTdsA.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTdsA.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTdsA.begin();  //initialization

  pinMode (relay1, OUTPUT);
  pinMode (relay2, OUTPUT);
  pinMode (relay3, OUTPUT);
  pinMode (relay4, OUTPUT);
  pinMode (relay5, OUTPUT);
  pinMode (relay6, OUTPUT);

  digitalWrite (relay1, HIGH);
  digitalWrite (relay2, HIGH);
  digitalWrite (relay3, HIGH);
  digitalWrite (relay4, HIGH);
  digitalWrite (relay5, HIGH);
  digitalWrite (relay6, HIGH);
  
  Serial3.begin(9600);
  WiFi.init(&Serial3);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  
  printWifiStatus();
}

void loop()
{  
  tmElements_t tm;

  RTC.read(tm);

  jam = tm.Hour;

  //Serial.println (jam);

  if (jam == 7 && state == 0){ // set hidupkan lampu 
  digitalWrite (relay1, LOW);
  digitalWrite (relay2, LOW);
  digitalWrite (relay3, LOW);
  digitalWrite (relay4, LOW);
  digitalWrite (relay5, LOW);
  digitalWrite (relay6, LOW);
  state = 1;
    }

  if (jam == 19 && state == 1){ // set matikan lampu
  digitalWrite (relay1, HIGH);
  digitalWrite (relay2, HIGH);
  digitalWrite (relay3, HIGH);
  digitalWrite (relay4, HIGH);
  digitalWrite (relay5, HIGH);
  digitalWrite (relay6, HIGH);
  state = 0;
  }
  
  tdssensor();

  ldrsensor();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  datatemp = String(t);
  datahum = String(h);
  datatds = String(tdsValue);
  dataL1 = String(valueldr1);
  dataL2 = String(valueldr2);
  dataL3 = String(valueldr3);
  dataL4 = String(valueldr4);
  dataL5 = String(valueldr5);
  dataL6 = String(valueldr6);
  
  
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

    httpRequest1();
    delay (3000);
    httpRequest2();
    delay (3000);

}


void httpRequest1()
{
  Serial.println();
  client.stop();

  if (client.connect(server, 80)) {
    Serial.println("Connecting...");
    
    client.println("GET /update?api_key=BAJ9ZS44H54ND60H&field1=" + String(datatemp) + "&field2=" + String(datahum) + "&field3=" + String(datatds) + " HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
  }
  else {
    Serial.println("Connection failed");
  }
}

void httpRequest2()
{
  Serial.println();
  client.stop();

  if (client.connect(server, 80)) {
    Serial.println("Connecting...");

    client.println("GET /update?api_key=TM8W0F1OY01Q7K5Q&field1=" + String(dataL1) + "&field2=" + String(dataL2) + "&field3=" + String(dataL3) + "&field4=" + String(dataL4) + "&field5=" + String(dataL5) + "&field6=" + String(dataL6) + " HTTP/1.1");
    client.println("Host: api.thingspeak.com");
    client.println("Connection: close");
    client.println();

    lastConnectionTime = millis();
  }
  else {
    Serial.println("Connection failed");
  }
}


void tdssensor(){
  gravityTdsA.setTemperature(temperature);  
  gravityTdsA.update();  
  tdsValue = gravityTdsA.getTdsValue();  
  }

void ldrsensor(){
  valueldr1 = analogRead (ldr1);
  valueldr2 = analogRead (ldr2);
  valueldr3 = analogRead (ldr3);
  valueldr4 = analogRead (ldr4);
  valueldr5 = analogRead (ldr5);
  valueldr6 = analogRead (ldr6); 
  }

void printWifiStatus()
{
  
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
