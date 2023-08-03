# CANaBus

CANaBus is a set of software and hardware meant to extract CAN messages and create useful gadgets. The goal is to read CAN messages from a bus, extract the CAN-ID and Data and broadcast them over ESPNow. The gadgets are ESPnow receivers that filter and process the CAN-IDs and data to do something useful them.

## The Gateway

The gateway is made of an ESP32 Dev Kit 1 board coupled with my CAN bus shield connected directly to the car's can bus ( usually though the OBDII port). To install this hardware in my car, I needed to do some modifications to make it safer to be connected permanently and create its power usage to use a little power as possible in stand by mode. 

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

![ESP32 Shield](https://i9.ytimg.com/vi/Se2KCVyD7CM/mqdefault.jpg?v=63d1884b&sqp=CIyW1p8G&rs=AOn4CLCGY11Qj282vpsAp2_cl1cp5jpzcw)

<figure class="video_container">
  <iframe src="https://youtu.be/Se2KCVyD7CM" frameborder="0" allowfullscreen="true"> </iframe>
</figure>

## The Gadgets

### Garage door opener

This gadget waits for 2 consecutive high beam signals within 1 second and triggers a relay to open or close a garage door opener. 

### SLCAN/LAWICEL & SavvyCAN

A recevier that convert the ESPNow CAN messages into SLCAN/LAWICEL to allow it to be used with SavvyCAN for sniffing, monitoring, analyzing and decoding. 

![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/savvycan.png)

### Coming soon

- Tesla's gauges
- GPS tracker
- Bus Logger

![SavvyCAN](https://gitlab.com/MrDIYca/canabus/-/raw/main/img/gadget_tesla_screen.png)

### Barebone

A skeleton barebone gadget to get started. 

