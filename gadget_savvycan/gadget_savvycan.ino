/*  ============================================================================

     MrDIY.ca CAN Project

     GADGET: SLCAN/LAWICEL

     https://www.canusb.com/files/can232_v3.pdf

     For ESP8266/Wemos D1 Mini (https://s.click.aliexpress.com/e/_Af36zD)

  ============================================================================= */

//#define ENALBE_FPS 1

#include <ESP8266WiFi.h>
#include <espnow.h>


#define SERIAL_MAX_BUFF_LEN   30

typedef struct esp_frame_t {
  int mesh_id;
  unsigned long can_id;
  byte can_len;
  uint8_t can_data[8];
};

bool          send_can_msgs = true;
bool          send_timestamp = true;
bool          got_new_can_msg = false;
bool          send_OK = true;
uint8_t       buff[40];
char          slcan_com[SERIAL_MAX_BUFF_LEN];
esp_frame_t   esp_frame;

#ifdef ENALBE_FPS
volatile uint32_t frameCounter = 0;  // Counter for received frames
uint32_t lastFpsCheck = 0;           // Last time we checked fps
uint32_t fps = 0;                    // Frames per second
#endif

/* ================================== Setup ======================================== */

void setup() {

  Serial.begin(115200);
  Serial.setTimeout(1);
  WiFi.mode(WIFI_STA);
  while (esp_now_init() != 0) {}
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

/* ================================== Loop ======================================== */

void loop() {

  sendCanOverSerial();
  handleSerial();

#ifdef ENALBE_FPS
  if (millis() - lastFpsCheck >= 1000) {
    fps = frameCounter;  // Current fps is the frame counter value
    frameCounter = 0;    // Reset the frame counter
    lastFpsCheck = millis();  // Update the last check time
    Serial.print("FPS: ");
    Serial.println(fps);
  }
#endif
}

/* ================================ ESPNow receiver ================================= */

void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {

#ifdef ENALBE_FPS
  frameCounter++;
#endif
  if ( got_new_can_msg ) return;
  memcpy(&esp_frame, incomingData, sizeof(esp_frame));
  got_new_can_msg = true;   // TODO: queue the msgs
}

/* ============================ CAN to SLCAN/LAWICEL ================================= */

void sendCanOverSerial() {

#ifdef ENALBE_FPS
  return;
#endif
  if (got_new_can_msg ) {

    char outputBuff[45]; // Increased size for safety
    int idx = 0;

    /*
        if (esp_frame.can_id <= 0x7FF) {
          // Standard 11-bit CAN ID
          idx += snprintf(outputBuff + idx, sizeof(outputBuff) - idx, "t%03x", esp_frame.can_id);
        } else if (esp_frame.can_id <= 0x1FFFFFFF) {
          // Extended 29-bit CAN ID
          idx += snprintf(outputBuff + idx, sizeof(outputBuff) - idx, "T%08x", esp_frame.can_id);
        } else {
          // Invalid CAN ID
          return;
        }
    */
    idx += snprintf(outputBuff + idx, sizeof(outputBuff) - idx, "t%03x", esp_frame.can_id);
    idx += snprintf(outputBuff + idx, sizeof(outputBuff) - idx, "%d", esp_frame.can_len);
    for (int i = 0; i < esp_frame.can_len; i++) {
      idx += snprintf(outputBuff + idx, sizeof(outputBuff) - idx, "%02x", esp_frame.can_data[i]);
    }

    if (send_timestamp) {
      
      uint16_t timestamp = (uint16_t) millis() % 60000; // Wrap around every minute
      idx += snprintf(outputBuff + idx, sizeof(outputBuff) - idx, "%02x", slcan_get_time());
    }
    
    outputBuff[idx++] = '\r';     // Carriage return
    outputBuff[idx++] = '\0';    // Null-terminate the buffer
    Serial.print(outputBuff);

    got_new_can_msg = false;
  }
}

unsigned short slcan_get_time() {

  unsigned long currentTime = millis();
  unsigned short timestamp = static_cast<unsigned short>(currentTime % 60000);
  return timestamp;
}

/* ============================ SLCAN/LAWICEL Commands ====================================

           ACK = Acknoledged but not implemented ( OK returned)
           Yes = implemented
            -  = Not implemented


      CMD | IMPLEMENTED | SYNTAX               | DESCRIPTION
      ------------------------------------------------------------------------------------------------------------
      'S' |   ACK       |   Sn[CR]               The CAN Gateway cannot be changed over this tool
          |             |                        Setup with standard CAN bit-rates where n is 0-8.
          |             |                        S0 10Kbit,S4 125Kbit, S8 1Mbit, S1 20Kbit, S5 250Kbit, S9 83.3Kbit, S2 50Kbit, S6 500Kbit, S3 100Kbit, S7 800Kbit
      's' |   ACK       |   sxxyy[CR]            Setup with BTR0/BTR1 CAN bit-rates where xx and yy is a hex value.
      'O' |   ACK       |   O[CR]                Open the CAN channel in normal mode (sending & receiving).
      'L' |   ACK       |   L[CR]                Open the CAN channel in listen only mode (receiving).
      'C' |   YES       |   C[CR]                Close the CAN channel.
      't' |   YES       |   tiiildd...[CR]       Transmit a standard (11bit) CAN frame.
      'T' |   ACK       |   Tiiiiiiiildd...[CR]  Transmit an extended (29bit) CAN frame
      'r' |   ACK       |   riiil[CR]            Transmit an standard RTR (11bit) CAN frame.
      'R' |   ACK       |   Riiiiiiiil[CR]       Transmit an extended RTR (29bit) CAN frame.
      'P' |   ACK       |   P[CR]                Poll incomming FIFO for CAN frames (single poll)
      'A' |   ACK       |   A[CR]                Polls incomming FIFO for CAN frames (all pending frames)
      'F' |   ACK       |   F[CR]                Read Status Flags: bit 0 = RX Fifo Full, 1 = TX Fifo Full, 2 = Error warning, 3 = Data overrun, 5= Error passive, 6 = Arb. Lost, 7 = Bus Error
      'X' |   ACK       |   Xn[CR]               Sets Auto Poll/Send ON/OFF for received frames.
      'W' |    -        |   Wn[CR]               Filter mode setting. By default CAN232 works in dual filter mode (0) and is backwards compatible with previous CAN232 versions.
      'M' |    -        |   Mxxxxxxxx[CR]        Sets Acceptance Code Register (ACn Register of SJA1000). // we use MCP2515, not supported
      'm' |    -        |   mxxxxxxxx[CR]        Sets Acceptance Mask Register (AMn Register of SJA1000). // we use MCP2515, not supported
      'U' |   ACK       |   Un[CR]               Setup new UART baud rate n=0-6: 0-230400, 1:115200, 2:57600, 3:38400, 4:19200, 5:9600, 6:2400
      'V' |   YES       |   v[CR]                Get Version number of both CAN232 hardware and software
      'v' |   YES       |   V[CR]                Get Version number of both CAN232 hardware and software
      'N' |   YES       |   N[CR]                Get Serial number of the CAN232.
      'Z' |   YES       |   Zn[CR]               Sets Time Stamp ON/OFF for received frames only. EXTENSION to LAWICEL: Z2 - millis() timestamp w/o standard 60000ms cycle
      'Q' |    -        |   Qn[CR]               Auto Startup feature (from power on).

*/

void handleSerial() {

  if (Serial.available() == 0) return;

  Serial.readBytesUntil(13, slcan_com, SERIAL_MAX_BUFF_LEN);

  switch (slcan_com[0] ) {
    case 'S':
    case 's':
    case 'R':
    case 'r':
    case 'P':
    case 'A':
    case 'x':
    case 'U':
      //Serial.begin
      break;

    case 't':
      Serial.print("z");
      break;

    case 'T':
      Serial.print("Z");
      break;

    case 'O':
    case 'L':
      send_can_msgs = true;
      break;

    case 'C':
      send_can_msgs = false;
      break;

    case 'F':
      Serial.print("F00");
      break;

    case 'V':
    case 'v':
      Serial.print("V1013\n");
      break;

    case 'N':
      Serial.print("NA123\n");
      break;

    case 'B':
      Serial.print("CAN1");
      Serial.print("\n");
      break;

    case 'Z':
      if ( slcan_com[1] == '1') send_timestamp = true;
      if ( slcan_com[1] == '0') send_timestamp = false;
      break;

    case 'X':
      break;

    default:
      send_OK = false;
      break;

  }
  if ( send_OK ) send_ack();
  send_OK = true;
}

void send_ack() {
  Serial.write('\r'); // ACK
}

void send_nack() {
  Serial.write('\a'); // NACK
}