#include <ESP8266WiFi.h>
#include <espnow.h>
#include <user_interface.h> 

#define STATUS_LED 2 

uint8_t broadcastAddress[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

typedef struct __attribute__((packed)) {
  uint8_t ttl;      
  char json[250];    
} Packet;

Packet incoming;

unsigned long ledOffTime = 0;
const int blinkDuration = 80;

void OnDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  digitalWrite(STATUS_LED, LOW);
  ledOffTime = millis() + blinkDuration;

  if (len == 250) {
    incoming.ttl = 5;
    memcpy(incoming.json, data, 250);
  } 
  else if (len == sizeof(Packet)) {
    memcpy(&incoming, data, sizeof(incoming));
  } 
  else {
    Serial.print("Dropped: Invalid packet size. Received ");
    Serial.println(len);
    return;
  }

  if (incoming.ttl <= 1) {
    return;
  }

  
  incoming.ttl--; 

  
  Serial.println(incoming.json);
  
  esp_now_send(broadcastAddress, (uint8_t*)&incoming, sizeof(incoming));
}

void updateLED() {
  if (millis() >= ledOffTime && digitalRead(STATUS_LED) == LOW) {
    digitalWrite(STATUS_LED, HIGH); 
  }
}

void setup() {
  Serial.begin(115200);
  
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH); 

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  wifi_set_channel(1);

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(OnDataRecv);
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  Serial.println("Bridge node initialized...");
}

void loop() {
  updateLED();
}
