/*  ============================================================================

     MrDIY - CAN Gateway (OBDII with Power Saving Modes)
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

#define SHIELD_LED_PIN           26
#define SHIELD_CAN_RX            GPIO_NUM_5
#define SHIELD_CAN_TX            GPIO_NUM_4
#define SHIELD_VOLTAGE_DIVIDER   32  /* the jumper must be soldered on the v1.1 shield. Voltage divider doesn't work/exist on v1.0 */

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
esp_frame_t          esp_frame;
esp_now_peer_info_t  peerInfo;
bool espnow_is_on    = false;

unsigned long     cutoff_timestamp = 0;
#define           CAN_IDLE_TIMEOUT   3000     /* time esp32 wait before going to sleep after on CAN avitivitesin ( in milliseconds) */
#define           SLEEP_PERIOD        5        /* Time ESP32 will go to sleep (in seconds) */

#define CAR_IS_RUNNING   1
#define CAR_IS_OFF       0
int car_status           = CAR_IS_OFF;
/* ================================== Setup ======================================== */

void setup() {

#ifdef DEBUG_FLAG
  Serial.begin(115200);
#endif

  debugln("");
  debugln("------------------------");
  debugln("    MrDIY CAN SHIELD");
  debugln("------------------------");
  debugln(" CAN...............INIT");

  CAN0.setCANPins(GPIO_NUM_5, GPIO_NUM_4);
  CAN0.setListenOnlyMode(true);
  if (!CAN0.begin(500000)) {
    debugln(" CAN...............FAIL");
    ESP.restart();
  }
  debugln(" CAN.................OK");
  CAN0.watchFor();

  pinMode(SHIELD_LED_PIN, OUTPUT);
  digitalWrite(SHIELD_LED_PIN, LOW);

  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason != ESP_SLEEP_WAKEUP_TIMER ) cutoff_timestamp = millis() + CAN_IDLE_TIMEOUT * 2;
  else cutoff_timestamp = millis() + 5;
}

/* ================================== Loop ======================================== */

void loop() {

  CAN_FRAME can_message;

  if (CAN0.read(can_message)) {

    debugln(" CAN ................ IN");

    if (can_message.id == 0) return;
    enable_ESPNow();
    cutoff_timestamp = millis() + CAN_IDLE_TIMEOUT;
    debugln(" CAN ............... EXT");
    esp_frame.mesh_id = MESH_ID;
    esp_frame.can_id = can_message.id;
    esp_frame.len = can_message.length;
    for (int i = 0; i < can_message.length; i++) {
      esp_frame.d[i] = can_message.data.byte[i];
    }
    esp_now_send(broadcastAddress, (uint8_t *)&esp_frame, sizeof(esp_frame));

#ifdef DEBUG_FLAG
    printCAN(&esp_frame);
    printFrame(&can_message);
#endif
  } else if ( millis() > cutoff_timestamp ) esp_deep_sleep();

  update_car_status();

}

/* ================================== Power ======================================== */

void enable_ESPNow() {

  if ( espnow_is_on == true) return;
  debugln(" ESPNOW............INIT");
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {

    digitalWrite(SHIELD_LED_PIN, LOW);
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
  digitalWrite(SHIELD_LED_PIN, HIGH);
  espnow_is_on = true;
  debugln(" ESPNOW........... GOOD");
}

void esp_deep_sleep() {

  esp_sleep_enable_timer_wakeup(SLEEP_PERIOD * 1000000);
  debugln(" ESP32........... SLEEP");
  esp_deep_sleep_start();
}

void update_car_status() {

  /* currently not doing anything, simply updating the car_status and turning on/off the LED */
  int avg_voltage = get_vin_voltage();
  
  if ( avg_voltage > 3500) {
    car_status = CAR_IS_RUNNING;
    digitalWrite(SHIELD_LED_PIN, HIGH);

  } else if ( avg_voltage > 3250) {
    // gray area, do nothing

  } else {
    car_status = CAR_IS_OFF;
    digitalWrite(SHIELD_LED_PIN, LOW);
    // if( avg_voltage  < 3150 ) battery is less than 70% - danger area, good idea to put ESP32 to sleep
  }
}

double get_vin_voltage() {

  /*
    MEASSURED ADC VALUES for a given voltage
      Volt    ADC 
      10.5    2640
      11.31   2850
      11.58   2930
      11.75   2970
      11.9    3000
      12.06   3060
      12.2    3100
      12.32   3150
      12.42   3170
      12.5    3190
      12.6    3220 ---- max 3250 for car to be off
      |||||||||||||||||  gray area   |||||||||||||||
      13.5    3520 ---- min 3500 for car to be running
      14.7    4030
  */

  // 10 samples = 2ms, 50 samples = 10ms
  int average = 0;
  for (int i = 0; i < 10; i++) average = average + analogRead(SHIELD_VOLTAGE_DIVIDER);
  average = average / 10;
  return average;
}


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