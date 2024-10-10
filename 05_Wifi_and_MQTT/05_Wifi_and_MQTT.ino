/*
 *     Maker      : DIYSensors                
 *     Project    : Connect to WiFi and MQTT
 *     Version    : 1.0
 *     Date       : 08-2024
 *     Programmer : Ap Matteman
 */    

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "arduino_secrets.h"

int iWiFiTry = 0;          
int iMQTTTry = 0;
String sClient_id;

int iCount;   // For DEMO

const char* ssid = YourSSID;
const char* password = YourWiFiPassWord;
const char* HostName = "WiFi_and_MQTT";

const char* mqtt_broker = YourMQTTserver;
const char* mqtt_user = YourMQTTuser;
const char* mqtt_password = YourMQTTpassword;

WiFiClient espClient;
PubSubClient MQTTclient(espClient); // MQTT Client

void setup()   {   
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }             
  Connect2WiFi();
  Connect2MQTT();
  iCount = 0;
}

void Connect2WiFi() { 
  //Connect to WiFi
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


  delay(1000);


  // If you want to reboot your device when WiFi is not working
  if (iWiFiTry > 10){
    Serial.println("REBOOTING");
    Serial.println(" Reboot in 2 seconds");
    Serial.println(""); 
    delay(2000);
    ESP.restart(); 
  }

 
}