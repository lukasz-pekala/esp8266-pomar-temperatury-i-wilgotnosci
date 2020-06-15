#include <DHT.h>

#include "Esp8266Pins.h"

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <CircularBuffer.h>



//======================Common
String convertFloatToString(float temperature)
{ // begin function

  char temp[10];
  String tempAsString;
    
  // perform conversion
  dtostrf(temperature, 1, 2, temp);
    
    // create string object
  tempAsString = String(temp);
  
  return tempAsString;
  
}

//======================Common


//======================DHT section

#define DHTPIN 12         // wyjście danych sensora jest dołączone do linii D6 nodemcu
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);


///=====================Data structures
float temperature;
float humidity;

// TODO: Change to struct if possible
CircularBuffer<float, 400> temperatures;   // uses 988 bytes per 100
CircularBuffer<float, 400> humidities;   // uses 988 bytes per 100
CircularBuffer<String, 400> utcTimes;   //
CircularBuffer<unsigned long, 400> epochs;  //
//=====================Data structures


//======================DHT section


//======================TIME

const int second = 1000;
const int minute = 60 * second;
int period = 10 * minute;
unsigned long time_now = 0;
//======================TIME


//======================


//======================NTP

const long utcOffsetInSeconds = 0;

//char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffsetInSeconds);

//======================NTP



//======================Web server section

#ifndef STASSID
#define STASSID "<YOUR_SSID>"
#define STAPSK  "<YOUR_PASSWORD>"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

void handleRoot() {
  digitalWrite(Esp8266Pins::BuiltinLed, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(Esp8266Pins::BuiltinLed, 0);
}

void handleNotFound() {
  digitalWrite(Esp8266Pins::BuiltinLed, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(Esp8266Pins::BuiltinLed, 0);
}

void dhtData() {
  digitalWrite(Esp8266Pins::BuiltinLed, 1);
  timeClient.update();
  server.send(200, "text/json", "{\"t\":\"" + convertFloatToString(temperature) + "\", \"h\":\"" + convertFloatToString(humidity) + "\", \"utcTime:\"" + timeClient.getFormattedTime() + "\" }");
  digitalWrite(Esp8266Pins::BuiltinLed, 0);
}

String getJsonEntry(int i) {
  //return "{\"t\":\"" + convertFloatToString(temperatures[i]) + "\", \"h\":\"" + convertFloatToString(humidities[i]) + "\", \"utcTime\":\"" + utcTimes[i] + "\" }";
  return "{\"t\":\"" + convertFloatToString(temperatures[i]) + "\", \"h\":\"" + convertFloatToString(humidities[i]) + "\", \"utcTime\":\"" + utcTimes[i] + "\", \"epoch\":\"" + epochs[i] + "\"}";
}

void temperaturesRoute() {
  digitalWrite(Esp8266Pins::BuiltinLed, 1);
  //timeClient.update();
  String json = "{\"data\": [";
  
  // the iterator variable i is of the correct type, even if  
  // we don't know what's the buffer declared capacity
  for (decltype(temperatures)::index_t i = 0; i < temperatures.size() - 1; i++) {
    json += getJsonEntry(i) + ",";
  }
  json += getJsonEntry(temperatures.size() - 1) + "]}";
  
  server.send(200, "text/json", json);
  digitalWrite(Esp8266Pins::BuiltinLed, 0);
}


void dataRoute() {
  digitalWrite(Esp8266Pins::BuiltinLed, 1);

  float mean = 0;
  float min = 100;
  decltype(temperatures)::index_t minIndex;
  decltype(temperatures)::index_t maxIndex;
  float max = -100;

  // the iterator variable i is of the correct type, even if  
  // we don't know what's the buffer declared capacity
  
  for (decltype(temperatures)::index_t i = 0; i < temperatures.size(); i++) {
    if (temperatures[i] < min) {
      min = temperatures[i];
      minIndex = i;
    }
    if (temperatures[i] > max) {
      max = temperatures[i];
      maxIndex = i;
    }
    mean += temperatures[i];
  }
  mean = mean / temperatures.size();
  String json = "{\"mean\":" + convertFloatToString(mean) + ",\"data\": [";
  json += getJsonEntry(minIndex) + ",";
  json += getJsonEntry(maxIndex) + "]}";
  
  server.send(200, "text/json", json);
  digitalWrite(Esp8266Pins::BuiltinLed, 0);
}

//======================Web server section



void setup()
{
  Serial.begin(115200);
  
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/dht", dhtData);

  server.on("/data", temperaturesRoute);
  
  server.on("/stats", dataRoute);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  timeClient.begin();

  Serial.println("Warming up DHT sensor");
  Serial.println(dht.readTemperature());
  Serial.println(dht.readHumidity());
  Serial.println(timeClient.getFormattedTime());
  Serial.println(timeClient.getEpochTime());
}

void loop()
{
  if(millis() >= time_now + period){
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    };
    
    time_now += period;
      
    temperatures.push(dht.readTemperature());
    humidities.push(dht.readHumidity());
    utcTimes.push(timeClient.getFormattedTime());
    epochs.push(timeClient.getEpochTime());
  
    if (isnan(temperature) || isnan(humidity))
    {
      Serial.println("ERROR: Blad odczytu danych z czujnika!");
    }
    else
    {
      Serial.print("Wilgotnosc: ");
      Serial.print(humidity);
      Serial.print(" % ");
      Serial.print("Temperatura: ");
      Serial.print(temperature);
      Serial.println(" *C");
    }   
  }
  
  server.handleClient();
  MDNS.update();             
}
