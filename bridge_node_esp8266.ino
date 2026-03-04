#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t broadcastAddress[] = {0xff,0xff,0xff,0xff,0xff,0xff};

typedef struct {
  char json[200];
} Packet;

Packet incoming;

void OnDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {

  memcpy(&incoming, data, sizeof(incoming));

  Serial.print("Received: ");
  Serial.println(incoming.json);

  esp_now_send(broadcastAddress,(uint8_t*)&incoming,sizeof(incoming));

  Serial.println("Rebroadcasted");
}

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  esp_now_register_recv_cb(OnDataRecv);

  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  Serial.println("Bridge node started");
}

void loop() {
}
