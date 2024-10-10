/*
 *     Maker      : DIYSensors                
 *     Project    : Connect to WiFi and MQTT with OTA
 *     Version    : 1.0
 *     Date       : 10-2024
 *     Programmer : Ap Matteman
 *    
 */    


#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

int iWiFiTry = 0;          
int iMQTTTry = 0;
String sClient_id;

int iCount;   // For DEMO

unsigned long lPmillis = 0;        // will store last time MQTT has published
const long lInterval = 1000; // 1000 ms => 1 Second

const char* ssid = YourSSID;
const char* password = YourWiFiPassWord;
const char* HostName = "Arduino_OTA_DEMO";

const char* mqtt_broker = YourMQTTserver;
const char* mqtt_user = YourMQTTuser;
const char* mqtt_password = YourMQTTpassword;

WiFiClient espClient;
PubSubClient MQTTclient(espClient); // MQTT Client

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }  // wait for serial port to connect. Needed for native USB port only           
  Connect2WiFi();
  Connect2MQTT();
  iCount = 0;
}

void Connect2WiFi() { 
  //Connect to WiFi
  // WiFi.mode(WIFI_STA);  //in case of an ESP32
  iWiFiTry = 0;
  WiFi.begin(ssid, password);
  WiFi.setHostname(HostName);
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED && iWiFiTry < 11) { //Try to connect to WiFi for 11 times
    ++iWiFiTry;
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  // ArduinoOTA.setPort(8266); // Port defaults to 8266
  
  ArduinoOTA.setHostname(HostName); 
 // ArduinoOTA.setPassword((const char *)OTAPassword); // Optional password

  ArduinoOTA.onStart([]() { Serial.println("Start"); });
  ArduinoOTA.onEnd([]()   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("Ready");

  //Unique MQTT Device name
  sClient_id = "esp-client-" + String(WiFi.macAddress());
  Serial.print("ESP Client name: "); Serial.println(sClient_id);
}

void Connect2MQTT() {
  // Connect to the MQTT server
  iMQTTTry=0;
  if (WiFi.status() != WL_CONNECTED) { 
    Connect2WiFi; 
  }

  Serial.print("Connecting to MQTT ");
  MQTTclient.setServer(mqtt_broker, 1883);
  while (!MQTTclient.connect(sClient_id.c_str(), mqtt_user, mqtt_password) && iMQTTTry < 11) { //Try to connect to MQTT for 11 times
    ++iMQTTTry;
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
}

void loop() {
  ArduinoOTA.handle();
  unsigned long lCmillis = millis();

  if (lCmillis - lPmillis >= lInterval) {
    lPmillis = lCmillis;
    if (!MQTTclient.connect(sClient_id.c_str(), mqtt_user, mqtt_password)) {
      Connect2MQTT();
    }

    Serial.println("Begin Loop");    
    MQTTclient.publish("DEMO/iCount", String(iCount).c_str());
    Serial.println(iCount);
    ++iCount;

    if(iCount>99) {
      iCount = 0;
    }

    // If you want to reboot your device when WiFi is not working
    if (iWiFiTry > 10){
      Serial.println("REBOOTING");
      Serial.println(" Reboot in 2 seconds");
      Serial.println(""); 
      delay(2000);
      ESP.restart(); 
    }
  }
  
}