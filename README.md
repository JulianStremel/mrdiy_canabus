This project comprises a combination of software and hardware components designed to extract Controller Area Network (CAN) messages and generate practical gadgets. The primary objective is to receive CAN messages from a bus, extract the CAN-ID and Data from them, and subsequently transmit this information over ESPNow. The gadgets serve as ESPNow receivers, responsible for filtering and processing the received CAN-IDs and data to perform specific useful tasks.

## The Gateway

The gateway consists of an ESP32 Dev Kit 1 board paired with a CAN bus shield, directly connected to the car's CAN bus, typically via the OBDII port. To install this hardware in my car securely, I performed certain modifications to ensure a safe permanent connection and minimize power consumption while in standby mode. See my full video below.

[![MrDIY Wireless Gateway YouTube video](https://img.youtube.com/vi/XiqU5wpnupk/0.jpg)](https://www.youtube.com/watch?v=XiqU5wpnupk)

<table>
<tr>

<td>
Power consumption while active: <100mA 
![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/power_usage_active.png)</td>
<td>
Power consumption while idle: <10 mA 
![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/power_usage_idle.png)
</td>

</tr>
</table>


The power reduction was made possible by putting the ESP32 in deep sleep when it stops detecting CAN messages and waking up every 5 seconds to check if the bus is active again. Two LEDs were desoldered from the ESP32 DevKit v1 and the shield. 

The CAN tranceiver's Rs pin was changed to connect to 3.3V instead of Gnd to put it in low power/ listen only mode. This helps with the power consumption and prevent it from sending any messages or flooding the bus by accident protecting the car while saving the battery.

## The Shield

A shield for the ESP32 Dev Kit 1 board that adds (1) a CAN bus transceiver and steps down the car's 12v down to a usable 3.3v

[![MrDIY CAN Shield YouTube video](https://img.youtube.com/vi/Se2KCVyD7CM/0.jpg)](https://www.youtube.com/watch?v=Se2KCVyD7CM)

<a href="https://store.mrdiy.ca/p/esp32-can-bus-shield/"> Get the ESP32 CAN Shield here</a>

![Schematic](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/schematic_can_shield__esp32_v1.2.png)


## The Gadgets

### Garage door opener

[WIP] This gadget waits for a signal from the car to trigger a relay to open or close a garage door. 

### SLCAN/LAWICEL & SavvyCAN

[WIP] This gadget converts the ESPNow CAN messages into SLCAN/LAWICEL format to allow it to be used with SavvyCAN for sniffing, monitoring, analyzing and decoding CAN messages. 

![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/savvycan.png)

### Other Gadgets

[WIP]

![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/gadget_tesla_screen.png)


