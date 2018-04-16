#include<WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <time.h>



#define DEBUG
#define FREQUENCYOFREADING .002e6 //The first number is number of seconds i.e.60

// Update these with values suitable for your network.
const char* SSID     = "REDCAP";
const char* PASSWORD = "";
const char* MQTT_SERVER = "mapmymotion.com";

//Comm Clients
WiFiClient espClient;
PubSubClient mqttClient(espClient);


void sendJS(int n, String sType[], float sValue[], String sUom[]) {
  String msg;

  //Get Current Epoch Time
  time_t currentTime = time(nullptr);
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  //root["oid"] = OID;  //Dont need this as the macid is added to the owners table
  //Get The MAC id and format it
  uint64_t chipid = ESP.getEfuseMac();
  char cMsg[16];
  snprintf (cMsg, 16, "%04X%8x", (uint16_t)(chipid >> 32), (uint32_t)chipid);
  root["macid"] = cMsg;
  // Set the time
  root["utime"] = currentTime;

  //Create the array of info
  JsonArray& readings = root.createNestedArray("readings");

  for (int i = 0; i < n; i++) {
    JsonObject& reading = readings.createNestedObject();
    reading["type"] = sType[i];
    reading["value"] = sValue[i];
    reading["uom"] = sUom[i];
  }
  root.printTo(msg);
  String strs = "TT/" + String(cMsg);
  //Serial.println(msg.length());
  mqttClient.publish(strs.c_str(), msg.c_str());
}

