#include <SPI.h>
#include <RH_RF69.h>

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 868.0

  #define RFM69_CS      8
  #define RFM69_INT     3
  #define RFM69_RST     4
  #define LED           13


// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission


long previousMillis = 0;

#define PERIOD 1000

void setup() 
{
  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer

  pinMode(LED, OUTPUT);     
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);
  
  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);
}

void loop() {
  timeToBlink();
  checkRadio();
    
}

void checkRadio(){
  if (rf69.available()) {
      // Should be a message for us now   
      uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
      uint8_t len = sizeof(buf);
      if (rf69.recv(buf, &len)) {
        digitalWrite(LED, HIGH);
        delay(100);
        digitalWrite(LED, LOW);
    
        if (!len) return;
        buf[len] = 0;
        Serial.print("Received [");
        Serial.print(len);
        Serial.print("]: ");
        Serial.println((char*)buf);
        Serial.print("RSSI: ");
        Serial.println(rf69.lastRssi(), DEC);
  
        if (strstr((char *)buf, "Hello World")) {
          // Send a reply!
          uint8_t data[] = "And hello back to you";
          rf69.send(data, sizeof(data));
          rf69.waitPacketSent();
          Serial.println("Sent a reply");
          //Blink(LED, 40, 3); //blink LED 3 times, 40ms between blinks
        }
      }
    }
}

void blink(){
    char radiopacket[20] = "Hello World #";
    itoa(packetnum++, radiopacket+13, 10);
    Serial.print("Sending "); Serial.println(radiopacket);
    
    // Send a message!
    rf69.send((uint8_t *)radiopacket, strlen(radiopacket));
    rf69.waitPacketSent();

    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);

    previousMillis = mills();
  
    // Now wait for a reply
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
  
    if (rf69.waitAvailableTimeout(500))  { 
      // Should be a reply message for us now   
      if (rf69.recv(buf, &len)) {
        Serial.print("Got a reply: ");
        Serial.println((char*)buf);
        //Blink(LED, 50, 3); //blink LED 3 times, 50ms between blinks
      }
    }
}

void timeToBlink(){
  long currentMillis = millis();

  //this next step is approximated to the nearest millisecond, even though the calulations may be more precise
  if(((currentMillis - previousMillis) >= (long) (PERIOD))){  //if the difference in time between the previousMillis and current time, is greater than or equal to the period of the firefly plus its modifier value; and the program has "started"
    blink();
  }
}

