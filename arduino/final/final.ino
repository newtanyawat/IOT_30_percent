#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <ESP8266mDNS.h>
#include "DHT.h"


const char* ssid = "MySweetHome_2.4G";
const char* password = "0921862948";

#define DHTPIN 2 
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Set LED GPIO
const int LEDWIFI = 5;
const int ledPin1 = 14;
const int ledPin2 = 12;
const int ledPin3 = 13;
const int ledPin4 = 15;
const int pirPin = 4;     
const int ledPin =  16;  

int pirState = 0;

String message = "";
String sliderValue1 = "0";
String sliderValue2 = "0";
String sliderValue3 = "0";
String sliderValue4 = "0";

float DHTCurrentValue1 = 0;
float DHTCurrentValue2 = 0;

int dutyCycle1;
int dutyCycle2;
int dutyCycle3;
int dutyCycle4;

//Json Variable to Hold Slider Values
JSONVar sliderValues;
JSONVar DHTValues;

//Get Slider Values
String getSliderValues(){
  sliderValues["sliderValue1"] = String(sliderValue1);
  sliderValues["sliderValue2"] = String(sliderValue2);
  sliderValues["sliderValue3"] = String(sliderValue3);
  sliderValues["sliderValue4"] = String(sliderValue4);

  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

String getDHTValues(){
  DHTValues["DHT_Humidity"] = String(DHTCurrentValue1);
  DHTValues["DHT_Temperature"] = String(DHTCurrentValue2);
  String jsonString = JSON.stringify(DHTValues);
  return jsonString;
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else{
   Serial.println("LittleFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void notifyClients(String sliderValues) {
  ws.textAll(sliderValues);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    if (message.indexOf("1s") >= 0) {
      sliderValue1 = message.substring(2);
      dutyCycle1 = map(sliderValue1.toInt(), 0, 100, 0, 1023);
      notifyClients(getSliderValues());
    }
    if (message.indexOf("2s") >= 0) {
      sliderValue2 = message.substring(2);
      dutyCycle2 = map(sliderValue2.toInt(), 0, 100, 0, 1023);
      notifyClients(getSliderValues());
    }    
    if (message.indexOf("3s") >= 0) {
      sliderValue3 = message.substring(2);
      dutyCycle3 = map(sliderValue3.toInt(), 0, 100, 0, 1023);
      notifyClients(getSliderValues());
    }
    if (message.indexOf("4s") >= 0) {
      sliderValue4 = message.substring(2);
      dutyCycle4 = map(sliderValue4.toInt(), 0, 100, 0, 1023);
      notifyClients(getSliderValues());
    }
    if (strcmp((char*)data, "getValues") == 0) {
      notifyClients(getSliderValues());
    }
    if (strcmp((char*)data, "DHT") == 0) {
      notifyClients(getDHTValues());
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void initDNS(){
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
}

void initPINMode(){
  pinMode(LEDWIFI, OUTPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
  }

void initWebServerRouting(){
  // Web Server Root URL  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
   String s = WiFi.localIP().toString().c_str();
   request->send(200, "application/json", s);
    
  });
  
  server.serveStatic("/", LittleFS, "/");
  }



void setup() {
  Serial.begin(115200);
  dht.begin();
  initPINMode();
  initFS();
  initWiFi();
  initDNS();
  initWebSocket();
  initWebServerRouting();
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
  digitalWrite(LEDWIFI, HIGH);
  Serial.println("---------------------- Start ----------------------");
}

void loop() {
  MDNS.update();
  analogWrite(ledPin1, dutyCycle1);
  analogWrite(ledPin2, dutyCycle2);
  analogWrite(ledPin3, dutyCycle3);
  analogWrite(ledPin4, dutyCycle4);
  // DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  DHTCurrentValue1 =  h;
  DHTCurrentValue2 =  t;
//
//  pirState = digitalRead(pirPin);
//  if (pirState == HIGH) {
//    Serial.println("---- PIR Detected ----");
//    delay(10000) ;
//  } else {
//    digitalWrite(ledPin, LOW);
//  }
  
  ws.cleanupClients();
}
