/*****************************************************
  ESP32 Touch Test and LED Ctrl
  Touch pin ==> Touch0 is T0 which is on GPIO 4 (D4).
  LED pin   ==> D2

  MJRoBot.org 6Sept17
*****************************************************/
#include "definitions.h"


#define TOUCH_PIN 4 // ESP32 Pin D4
#define LED_PIN 2

int touch_value = 100;
long lastMsg = 0;

void setup()
{
  Serial.begin(115200);
  // Get the current time
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");  //get the current time
  // Set up Wifi
  setup_wifi();
  // Set up MQTT
  setupMqtt();
  delay(1000); // give me time to bring up serial monitor
  Serial.println("ESP32 Touch Test");
  pinMode(LED_PIN, OUTPUT);
  digitalWrite (LED_PIN, LOW);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Used to create a random client id in reconnect();
  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Setup Mqtt
void setupMqtt() {
  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(callback);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  String x;
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    x += (char)payload[i];
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      mqttClient.publish("BARNES123", "Connected");
      // ... and resubscribe
      mqttClient.subscribe("BARNES123");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop()
{
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;
    touch_value = touchRead(TOUCH_PIN);
    Serial.println(touch_value);  // get value using T0
    long lastMsg = 0;

    if (touch_value < 50)
    {
      digitalWrite (LED_PIN, HIGH);
    }
    else
    {
      digitalWrite (LED_PIN, LOW);
    }
    int n = 1; //Number of data types to send
    String sType[n], sUOM[n];
    float sValue[n];
    for (int i = 0; i < n; i++) {
      if (i == 0) {
        sType[i] = 'M';
        sValue[i] = touch_value;
        sUOM[i] = 'R';
      }
    }
    sendJS(n, sType, sValue, sUOM);
  }
}
