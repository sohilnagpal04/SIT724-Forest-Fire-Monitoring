#include <SPI.h>
#include <RH_RF95.h>

// LoRa Pins for Feather M0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define RF95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

void setup() {
  Serial.begin(9600);
  delay(1000);

  // Reset LoRa module
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1);
  }

  rf95.setFrequency(RF95_FREQ);
  Serial.println("LoRa receiver ready");

  // Enable built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (rf95.available()) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      String received = String((char*)buf);
      Serial.println(received);  // Raspberry Pi script can parse this

      // Print RSSI and SNR
      int rssi = rf95.lastRssi();
      float snr = rf95.lastSNR();

      Serial.print("RSSI: ");
      Serial.println(rssi);
      Serial.print("SNR: ");
      Serial.println(snr);

      // Blink LED once after successful receive
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      Serial.println("Receive failed");
    }
  }
}
