/*  ============================================================================

     MrDIY - CAN Shield - https://youtu.be/gtVQeTRps7o

     Notes:
     - The Power/Red LED on the shield is not enabled my default. A jumper next to 
     the LED needs to be soldered
     - The voltage divider is not enabled by default on v1.1. A jumper on the back
     needs to be soldered
     - the ACT LED is connected to pin 26 (SHIELD_LED_PIN in the code)

    ATTENTION:
    ----------
     - If you are facing an assert failed error. Try getting the esp_can library 
     from my repo instead ( the file is called: libraries.zip on the root folder)
     or check https://github.com/collin80/esp32_can/issues/48

  ============================================================================= */
  

#include <esp32_can.h> /* https://github.com/collin80/esp32_can */

#define  SHIELD_LED_PIN   26

void setup()
{
  Serial.begin(115200);

  Serial.println("------------------------");
  Serial.println("    MrDIY CAN SHIELD");
  Serial.println("------------------------");

  Serial.println(" CAN...............INIT");
  CAN0.setCANPins(GPIO_NUM_5, GPIO_NUM_4);
  CAN0.begin(500000); // 500Kbps
  CAN0.watchFor();
  Serial.println(" CAN............500Kbps");

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