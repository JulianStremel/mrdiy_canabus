/*  ============================================================================

     MrDIY.ca CAN Project

     GADGET: Garage door opener

     Listen:     - Tucson: the cruise control button pressed for more than 5 seconds
                 - Tesla: the right wheel on the steering wheel pressed for more than 5 seconds

     Action:     sends an MQTT garage door opener ( throught the ESPNow to Wi-Fi gateway)

  ============================================================================================== */

#define DEBUG_FLAG 1

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

union sensor {
  unsigned  long long raw_ul;
  uint8_t d[8];
};

typedef struct esp_frame_t {
  int mesh_id;
  unsigned long can_id;
  byte len;
  uint8_t d[8];
};

esp_frame_t esp_frame;
sensor s;

// ------------- DBC ------------------------
/*
   TESLA
      BO_ 962 ID3C2VCLEFT_switchStatus: 8 VehicleBus
        SG_ VCLEFT_switchStatusIndex M : 0|2@1+ (1,0) [0|2] ""  Receiver
        SG_ VCLEFT_swcRightPressed m1 : 12|2@1+ (1,0) [0|3] ""  Receiver

   HUYNDAI
      BO_ 1265 CLU11: 4 CLU
        SG_ CF_Clu_CruiseSwMain : 3|1@1+ (1.0,0.0) [0.0|1.0] ""  EMS,LDWS_LKAS,SCC
*/

#define RIGHT_WHEEL_BUTTON_CAN_ID_TESLA     962   // 0x3C2
#define CRUISE_STATE_BUTTON_CAN_ID_TUCSON   1265  // 0x4F1  

#define BTN_TUCSON_CRUISE_UNPRESSED 0
#define BTN_TUCSON_CRUISE_PRESSED  1

#define BTN_TESLA_RIGHT_WHEEL_UNPRESSED 1
#define BTN_TESLA_RIGHT_WHEEL_PRESSED 2

int hb_status;
bool btn_is_held_on = false;
unsigned long btn_press_started;
#define REQUIRED_HOLD_DURATION_IN_SECONDS  3

// --------- GARAGE OPENER ------------------

#define GARAGE_RELAY_PIN   5      // D1
#define GARAGE_PRESS_DELAY 1000

void setup() {

#ifdef DEBUG_FLAG
  Serial.begin(115200);

  pinMode(GARAGE_RELAY_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(GARAGE_RELAY_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
#endif


  WiFi.mode(WIFI_STA);
  delay(10);
  debugln();
  debugln();
  debugln(F("=================================================================="));
  debugln(F("  Canabus System"));
  debugln(F("  GADGET: Garage door opener"));
  debugln(F("  mrdiy.ca"));
  debugln(F("=================================================================="));
  debugln();
  debugln(" ESPNOW ......... INIT");
  while (esp_now_init() != 0) {}
  debugln(" ESPNOW ........... OK");
  debugln("");
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
  btn_is_held_on = false;
  btn_press_started = 0;
}

void openGarageDoor() {

  digitalWrite(LED_BUILTIN, LOW);
  debugln(F("openGarage"));
  digitalWrite(GARAGE_RELAY_PIN, HIGH);
  delay(GARAGE_PRESS_DELAY);
  digitalWrite(GARAGE_RELAY_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {

  if (btn_press_started > 0 && millis() - btn_press_started > REQUIRED_HOLD_DURATION_IN_SECONDS * 1000) {
    openGarageDoor();
    btn_press_started = 0;
    btn_is_held_on = false;
  }
}

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {

  memcpy(&esp_frame, incomingData, sizeof(esp_frame));

  if ( esp_frame.can_id == RIGHT_WHEEL_BUTTON_CAN_ID_TESLA) {

    processRaw(&esp_frame, &s, 0, 2);
    if ( (int) s.raw_ul != 1 ) return; // ignore others in the mutiplex

    processRaw(&esp_frame, &s, 12, 2);
    hb_status = (int) s.raw_ul;

    if ( btn_is_held_on == false && hb_status == BTN_TESLA_RIGHT_WHEEL_PRESSED ) {
      btn_is_held_on = true;
      btn_press_started = millis();
    }

    if ( hb_status == BTN_TESLA_RIGHT_WHEEL_UNPRESSED ) {
      btn_is_held_on = false;
      btn_press_started = 0;
    }


  } else if ( esp_frame.can_id == CRUISE_STATE_BUTTON_CAN_ID_TUCSON) {

    processRaw(&esp_frame, &s, 3, 1);
    hb_status = (int) s.raw_ul;

    if ( btn_is_held_on == false && hb_status == BTN_TUCSON_CRUISE_PRESSED) {
      btn_is_held_on = true;
      btn_press_started = millis();
    }

    if ( hb_status == BTN_TUCSON_CRUISE_UNPRESSED ) {
      btn_is_held_on = false;
      btn_press_started = 0;
    }

  }
}

void processRaw(esp_frame_t *frame, sensor *s, int start_bit, int len) {

  for (int i = 0; i < 8 ; i++) s->d[i] = frame->d[i];
  s->raw_ul = s->raw_ul << (64 - start_bit - len)  >> (64 - len);
}