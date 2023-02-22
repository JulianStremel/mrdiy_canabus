#include <esp32_can.h> /* https://github.com/collin80/esp32_can */
#define  SHIELD_LED_PIN   26

void setup()
{
  Serial.begin(115200);
  CAN0.setCANPins(GPIO_NUM_5, GPIO_NUM_4);
  CAN0.setListenOnlyMode(true);
  CAN0.begin(500000); // 500Kbps
  CAN0.watchFor();
}


void loop()
{
  CAN_FRAME can_message;

  if (CAN0.read(can_message))
  {
    Serial.print("CAN MSG: 0x");
    Serial.print(can_message.id, HEX);
    Serial.print(" [");
    Serial.print(can_message.length, DEC);
    Serial.print("] <");
    for (int i = 0; i < can_message.length; i++)
    {
      if (i != 0) Serial.print(":");
      Serial.print(can_message.data.byte[i], HEX);
    }
    Serial.println(">");
  }
}