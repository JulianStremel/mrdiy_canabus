This project comprises a combination of software and hardware components designed to extract Controller Area Network (CAN) messages and generate practical gadgets. The primary objective is to receive CAN messages from a bus, extract the CAN-ID and Data from them, and subsequently transmit this information over ESPNow. The gadgets serve as ESPNow receivers, responsible for filtering and processing the received CAN-IDs and data to perform specific useful tasks.

## Install

Some of the fimrwares are available to be installed online directly to ESP32/ESP8266 using Chrome with one click - <a href='https://mrdiyca.gitlab.io/mrdiy-esp-online-flasher'>https://mrdiyca.gitlab.io/mrdiy-esp-online-flasher</a>

## The Gateway

The gateway consists of an ESP32 Dev Kit 1 board paired with a CAN bus shield (see below), directly connected to the car's CAN bus, typically via the OBDII port. See my full video below.

[![MrDIY Wireless Gateway YouTube video](https://img.youtube.com/vi/XiqU5wpnupk/0.jpg)](https://www.youtube.com/watch?v=XiqU5wpnupk)

<table>
<tr>

<td>
Power consumption while active: <100mA 
![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/power_usage_active.png)</td>
<td>
Power consumption while sleep with LEDs removed: <10 mA 
![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/power_usage_idle.png)
</td>

</tr>
</table>


The power reduction was made possible by putting the ESP32 in deep sleep when it stops detecting CAN messages and waking up every 5 seconds to check if the bus is active again. Two LEDs were desoldered from the ESP32 DevKit v1 and the shield. 

The CAN tranceiver's Rs pin was changed to connect to 3.3V instead of Gnd to put it in low power/ listen only mode. This helps with the power consumption and prevent it from sending any messages or flooding the bus by accident protecting the car while saving the battery.

## The Shield

I designed a shield for the ESP32 Dev Kit 1 board [<a href='https://geni.us/jS2Lx1W'>Amazon</a>, <a href='https://s.click.aliexpress.com/e/_DmhOymx'>AliExpress</a>] that adds a CAN bus transceiver and steps down the car's battery voltage down to a usable 3.3v.

In the upgraded (version 1.1) shield, I added a voltage divider on D32, a 120Ω CAN termination resistor and a physical toggle switch that allows the CAN transceiver to go in Listen-only / Do-Not-Transmit mode.

Get the ESP32 <a href="https://store.mrdiy.ca/p/esp32-can-bus-shield/"> CAN Shield here.</a>

[![MrDIY CAN Shield YouTube video](https://img.youtube.com/vi/Se2KCVyD7CM/0.jpg)](https://www.youtube.com/watch?v=Se2KCVyD7CM)


![Schematic](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/schematic_can_shield__esp32_v1.2.png)


## The Gadgets

### Garage door opener

This gadget waits for a signal from the car to trigger a relay to open or close a garage door. 

[![MrDIY Gadget: Garage Door Opener YouTube video](https://img.youtube.com/vi/PJhCqS9ZGLU/0.jpg)](https://www.youtube.com/watch?v=PJhCqS9ZGLU)

### SLCAN/LAWICEL & SavvyCAN

This gadget converts the ESPNow CAN messages into SLCAN/LAWICEL format to allow it to be used with SavvyCAN for sniffing, monitoring, analyzing and decoding CAN messages. 

[![MrDIY Gadget: SavvyCAN on ESP8266 YouTube video](https://img.youtube.com/vi/pontoaAcyZM/0.jpg)](https://www.youtube.com/watch?v=pontoaAcyZM)

### Other Gadgets

[WIP]

![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/gadget_tesla_screen.png)


