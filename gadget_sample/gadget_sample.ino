/*  ============================================================================

     MrDIY - CAN Gadget : sample
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

// ------------- ESPNow  ---------------------

#include <ESP8266WiFi.h>
#include <espnow.h>

#define MESH_ID   5555555

typedef struct esp_frame_t {
  int mesh_id;
  unsigned long can_id;
  byte len;
  uint8_t d[8];
};

esp_frame_t esp_frame;

void setup() {

#ifdef DEBUG_FLAG
  Serial.begin(115200);
#endif


  WiFi.mode(WIFI_STA);
  delay(10);
  debugln();
  debugln();
  debugln(F("=================================================================="));
  debugln(F("  MrDIY - CAN System"));
  debugln(F("  GADGET: Sample"));
  debugln(F("  mrdiy.ca"));
  debugln(F("=================================================================="));
  debugln();
  debugln(" ESPNOW ......... INIT");
  while (esp_now_init() != 0) {}
  debugln(" ESPNOW ........... OK");
  debugln("");
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {

  memcpy(&esp_frame, incomingData, sizeof(esp_frame));

  if (esp_frame.mesh_id == MESH_ID) {

    /* Got a CAN message with my ESPNow Mesh ID, do Something .... */

  }else{
    
    // messageis not for me, ignore!
  }
}