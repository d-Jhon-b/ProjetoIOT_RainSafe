#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include <WiFiClientSecure.h>

const char* ssid = "Wokwi-GUEST"; 
const char* password = "";
const char* mqtt_server = "cb1fe2a34743476eabeeaed802cf3e1e.s1.eu.hivemq.cloud"; 
const char* mqtt_user = "esp32_jhon"; 
const char* mqtt_pass = "C@os1945";
const int mqtt_port = 8883;
const char* mqtt_topic = "fatec/itaquera/grupo2/rainsafe";


// const char* mqtt_server = "broker.hivemq.com";

#define DHTPIN 15
#define DHTTYPE DHT22
#define LED_V1 18
#define LED_V2 19
#define LED_R1 21
#define LED_R2 22

DHT dht(DHTPIN, DHTTYPE);
// WiFiClient espClient;
WiFiClientSecure espClient;
PubSubClient client(espClient);


unsigned long lastMsg = 0;
bool sendingData = false;
void setup_wifi() {
  delay(10);
  Serial.println("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT Segura...");
    String clientId = "RainSafe-ESP32-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("conectado ao HiveMQ!");
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      
      for(int i=0; i<5; i++){
        digitalWrite(LED_R1, HIGH); digitalWrite(LED_R2, HIGH);
        delay(100);
        digitalWrite(LED_R1, LOW); digitalWrite(LED_R2, LOW);
        delay(100);
      }
      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_V1, OUTPUT); pinMode(LED_V2, OUTPUT);
  pinMode(LED_R1, OUTPUT); pinMode(LED_R2, OUTPUT);
  
  Serial.begin(115200);
  setup_wifi();
  
  espClient.setInsecure(); 
  
  client.setServer(mqtt_server, mqtt_port);
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  
  if (!sendingData) {
    digitalWrite(LED_V1, HIGH);
    digitalWrite(LED_V2, HIGH);
  }

  if (now - lastMsg > 10000) {
    lastMsg = now;
    sendingData = true;

    for(int i=0; i<3; i++) {
      digitalWrite(LED_V1, LOW); digitalWrite(LED_V2, LOW);
      delay(200);
      digitalWrite(LED_V1, HIGH); digitalWrite(LED_V2, HIGH);
      delay(200);
    }

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t)) {
      String status_chuva = (h > 80.0) ? "Alta Probabilidade" : "Baixo Risco";
      String payload = "{\"temp\": " + String(t) + ", \"umid\": " + String(h) + ", \"status\": \"" + status_chuva + "\"}";
      
      if(client.publish(mqtt_topic, payload.c_str())) {
        Serial.println("Dados enviados ao HiveMQ: " + payload);
        
        digitalWrite(LED_R1, HIGH); digitalWrite(LED_R2, HIGH);
        delay(3000);
        digitalWrite(LED_R1, LOW); digitalWrite(LED_R2, LOW);
      }
    }
    sendingData = false;
  }
}