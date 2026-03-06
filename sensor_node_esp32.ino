#include <WiFi.h>
#include <esp_now.h>

// Pin definitions
#define MQ2_PIN 32
#define VIBRATION_PIN 13
#define MQ135_PIN 35

uint8_t broadcastAddress[] = {0xff,0xff,0xff,0xff,0xff,0xff};

typedef struct {
  char json[200];
} Packet;

Packet packet;

void setup() {
  Serial.begin(115200);

  // Sensor pins
  pinMode(MQ2_PIN, INPUT);
  pinMode(VIBRATION_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);

  // WiFi & ESP-NOW setup
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
  esp_now_add_peer(&peerInfo);

  Serial.println("Sensor node started");
}

void loop() {
  // Read sensors
  int mq2Value = analogRead(MQ2_PIN);       // Gas level
  int mq135Value = analogRead(MQ135_PIN);   // Air quality
  int vibrationValue = digitalRead(VIBRATION_PIN); // 0 or 1

  // Format JSON
  snprintf(packet.json, sizeof(packet.json),
           "{\"node\":\"sensor\",\"mq2\":%d,\"mq135\":%d,\"vibration\":%d}",
           mq2Value, mq135Value, vibrationValue);

  // Send via ESP-NOW
  esp_now_send(broadcastAddress, (uint8_t*)&packet, sizeof(packet));

  Serial.println(packet.json);

  delay(2000);
}
