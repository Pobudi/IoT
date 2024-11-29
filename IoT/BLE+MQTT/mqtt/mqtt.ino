#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <esp_wifi.h>

const char* ssid = "vivo Y72 5G";
const char* password = "";

const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];
int value = 0;

const int ledPin = 2;
String mac;

void setup() {
  Serial.begin(115200);
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  for (int i=0; i<6; i++) {
    mac += baseMac[i];
  }
  pinMode(0, INPUT_PULLUP);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(ledPin, OUTPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
}

// tylko do demonstracji, nie potrzebne w projekcie
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageLed;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageLed += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "topic2") {
    Serial.print("Changing output to ");
    if(messageLed == "on"){
      Serial.println(messageLed);
      digitalWrite(ledPin, HIGH);
    }
    else if(messageLed == "off"){
      Serial.println(messageLed);
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("topic2");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (digitalRead(0) == LOW) {
    client.publish(mac+"/temp", 25);
    delay(500);
  }
}