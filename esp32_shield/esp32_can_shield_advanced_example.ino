/*  ============================================================================

     MrDIY - CAN Shield - https://youtu.be/gtVQeTRps7o

     THIS VERSION DO NOT USE the esp32_can LIBRARY.

     Notes:
     - The Power/Red LED on the shield is not enabled my default. A jumper next to
     the LED needs to be soldered
     - The voltage divider is not enabled by default on v1.1. A jumper on the back
     needs to be soldered
     - the ACT LED is connected to pin 26 (SHIELD_LED_PIN in the code)

     The API gives other possible speeds and alerts:
      https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/twai.html


  ============================================================================= */

#define DEBUG_FLAG  1

#ifdef DEBUG_FLAG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#include "driver/twai.h"
#define POLLING_RATE_MS 1000

#define SHIELD_LED_PIN GPIO_NUM_26
#define SHIELD_CAN_RX GPIO_NUM_5
#define SHIELD_CAN_TX GPIO_NUM_4
#define SHIELD_VOLTAGE_DIVIDER GPIO_NUM_32  /* the jumper must be soldered on the v1.1 shield. Voltage divider doesn't work/exist on v1.0. MAXIMUM 15 Volt Power input */


void setup() {
  
#ifdef DEBUG_FLAG
  Serial.begin(115200);
#endif

  debugln("");
  debugln("------------------------");
  debugln("    MrDIY CAN SHIELD");
  debugln("------------------------");

  if ( !initCAN()) {
    debugln(" SYSTEM......... FAIL");
    debugln(" ... restarting");
    delay(60 * 1000);
    ESP.restart();
  }
}

bool initCAN() {

  debugln(" CAN...............INIT");

  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)SHIELD_CAN_TX, (gpio_num_t)SHIELD_CAN_RX, TWAI_MODE_LISTEN_ONLY);
  g_config.rx_queue_len = 32;
  g_config.tx_queue_len = 2;  // not needed but here for testing

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();  // 500kbps
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    debugln(" CAN....Install..... OK");

  } else {
    debugln(" CAN....Driver... FAIL");
    return false;
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK) {
    debugln(" CAN....Start....... OK");

  } else {
    debugln(" CAN....Start..... FAIL");
    return false;
  }

  uint32_t alerts_to_enable = TWAI_ALERT_RX_DATA | TWAI_ALERT_ERR_PASS | TWAI_ALERT_BUS_ERROR | TWAI_ALERT_RX_QUEUE_FULL;
  if (twai_reconfigure_alerts(alerts_to_enable, NULL) == ESP_OK) {
    debugln(" CAN....Alerts...... OK");
  } else {
    debugln(" CAN....Alerts.... FAIL");
    return false;
  }
  debugln(" CAN.................OK");
  return true;
}


void loop() {

  uint32_t alerts_triggered;
  twai_read_alerts(&alerts_triggered, pdMS_TO_TICKS(POLLING_RATE_MS));

  if (alerts_triggered) {
    // got msgs
    if (TWAI_ALERT_RX_DATA) {
      twai_message_t message;
      while (twai_receive(&message, 0) == ESP_OK) handle_rx_message(message);
    }
  }
}

void handle_rx_message(twai_message_t& message) {

  if (message.extd) {
    Serial.print("CAX: 0x");
  } else {
    Serial.print("CAN: 0x");
  }
  Serial.print(message.identifier, HEX);
  Serial.print(" (");
  Serial.print(message.identifier, DEC);
  Serial.print(")");
  Serial.print(" [");
  Serial.print(message.data_length_code, DEC);
  Serial.print("] <");
  for (int i = 0; i < message.data_length_code; i++) {
    if (i != 0) Serial.print(":");
    Serial.print(message.data[i], HEX);
  }
  Serial.println(">");
}