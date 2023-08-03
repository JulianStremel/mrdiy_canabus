/*  ============================================================================

     MrDIY - CAN Gateway
     CAN Shield - https://youtu.be/gtVQeTRps7o

  ============================================================================= */

// ---- serial debug -------------------

// uncomment #define DEBUG_FLAG  1 to debug or troubleshoot issues
// leave it commented for normal use

#define DEBUG_FLAG  1

#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// --- shield pins ---------------------

#define SHIELD_LED_PIN 26
#define SHIELD_CAN_RX GPIO_NUM_5
#define SHIELD_CAN_TX GPIO_NUM_4

// --- can ---------------------------

#include <esp32_can.h>      /* https://github.com/collin80/esp32_can */
CAN_FRAME can_message;

// --- espnow ----------------------

#define MESH_ID 5555555
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
#include <esp_now.h>
#include <WiFi.h>

typedef struct esp_frame_t {
  int mesh_id;
  unsigned long can_id;
  byte len;
  uint8_t d[8];
};

esp_frame_t esp_frame;
esp_now_peer_info_t peerInfo;

/* ================================== Setup ======================================== */

void setup() {

#ifdef DEBUG_FLAG
  Serial.begin(1000000);
#endif

  pinMode(SHIELD_LED_PIN, OUTPUT);

  // flash the LED
  digitalWrite(SHIELD_LED_PIN, HIGH);

  debugln("");
  debugln("------------------------");
  debugln("    MrDIY CAN SHIELD");
  debugln("------------------------");
  debugln(" CAN...............INIT");

  /* setup CAN @500kbps */
  CAN0.setCANPins(GPIO_NUM_5, GPIO_NUM_4);
  if (!CAN0.begin(500000)) {
    debugln(" CAN...............FAIL");
    delay(500);
    ESP.restart();
  }
  debugln(" CAN.................OK");
  CAN0.watchFor();

  debugln(" ESPNOW............INIT");
  /* setup ESPNow */
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {

    debugln(" ESPNOW............ FAIL");
    debugln(" ... restarting");
    ESP.restart();
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    debugln(" ESPNOW............ FAIL");
    ESP.restart();
  }
  debugln(" ESPNOW..............OK");

  digitalWrite(SHIELD_LED_PIN, LOW);
}

/* ================================== Loop ======================================== */

void loop() {

  CAN_FRAME can_message;

  /* read the CAN msg */
  if (CAN0.read(can_message)) {

    //debugln(" CAN ................ IN");

    if (can_message.id == 0) return;
    //debugln(" CAN ............... EXT");
    /* prepare the ESPNow msg from the CAN msg */
    esp_frame.mesh_id = MESH_ID;
    esp_frame.can_id = can_message.id;
    esp_frame.len = can_message.length;
    for (int i = 0; i < can_message.length; i++) {
      esp_frame.d[i] = can_message.data.byte[i];
    }
    /* send the ESPNow msg */
    esp_now_send(broadcastAddress, (uint8_t *)&esp_frame, sizeof(esp_frame));

#ifdef DEBUG_FLAG
    printCAN(&esp_frame);
    //printFrame(&can_message);
#endif
  }
}

/* ================================== DEBUG ======================================== */

#ifdef DEBUG_FLAG
void printFrame(CAN_FRAME *message) {

  debug("CAN: 0x");
  Serial.print(message->id, HEX);
  debug(" (");
  Serial.print(message->id, DEC);
  debug(")");
  debug(" [");
  Serial.print(message->length, DEC);
  debug("] <");
  for (int i = 0; i < message->length; i++) {
    if (i != 0) debug(":");
    Serial.print(message->data.byte[i], HEX);
  }
  debugln(">");
}

void printCAN(esp_frame_t *message) {

  debug("ESP ");
  debug(message->mesh_id);
  debug(": ");
  debug(" 0x");
  Serial.print(message->can_id, HEX);
  debug(" (");
  Serial.print(message->can_id, DEC);
  debug(")");
  debug(" [");
  Serial.print(message->len, DEC);
  debug("] <");
  for (int i = 0; i < message->len; i++) {
    if (i != 0) debug(":");
    Serial.print(message->d[i], HEX);
  }
  debugln(">");
}
#endif