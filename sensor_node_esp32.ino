#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define MQ2_PIN 34
#define VIBRATION_PIN 13
#define MQ135_PIN 35
#define B_LED 12
#define DHTPIN 14
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

typedef struct {
  char json[250];
} Packet;

Packet packet;

void setup() {
  Serial.begin(115200);

  pinMode(MQ2_PIN, INPUT);
  pinMode(VIBRATION_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  pinMode(B_LED, OUTPUT);

  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("Sensor node with DHT11 started");
}

void loop() {
  // Reading sensors
  int mq2Value = analogRead(MQ2_PIN);   
  int mq135Value = analogRead(MQ135_PIN); 
  int vibrationValue = digitalRead(VIBRATION_PIN); 
  
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    h = 0;
    t = 0;
  }

  snprintf(packet.json, sizeof(packet.json),
           "{\"node\":\"sensor\",\"mq2\":%d,\"mq135\":%d,\"vib\":%d,\"temp\":%.1f,\"hum\":%.1f}",
           mq2Value, mq135Value, vibrationValue, t, h);

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&packet, sizeof(packet));
  
  digitalWrite(B_LED, HIGH);
  delay(200); 
  digitalWrite(B_LED, LOW);

  Serial.println(packet.json);
  delay(2000);
}
