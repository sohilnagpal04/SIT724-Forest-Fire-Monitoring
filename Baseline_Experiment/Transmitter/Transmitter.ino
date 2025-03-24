#include <SPI.h>
#include <RH_RF95.h>
#include <DHT.h>

// LoRa Pins
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// LoRa config
#define RF95_FREQ 915.0
#define TX_POWER 14
#define SPREADING_FACTOR 7

// DHT22
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Voltage divider pin
#define VOLTAGE_PIN A0

// LoRa driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Timing
unsigned long lastTransmit = 0;
unsigned long transmitInterval = 10000;
unsigned long startTime;

int packetID = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

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
  rf95.setTxPower(TX_POWER, false);
  rf95.setSpreadingFactor(SPREADING_FACTOR);
  
  dht.begin();
  startTime = millis();

  // Set built-in LED pin as output
  pinMode(LED_BUILTIN, OUTPUT);
}

float readVoltage() {
  int raw = analogRead(VOLTAGE_PIN);
  float measuredV = (raw * 3.3) / 1023.0; // Assuming 3.3V ADC ref
  float actualV = measuredV * (10.0 + 5.0) / 10.0; // Voltage divider: 10k & 5k
  return actualV;
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastTransmit >= transmitInterval) {
    lastTransmit = currentMillis;

    float temp = dht.readTemperature();
    float humid = dht.readHumidity();
    float voltage = readVoltage();
    unsigned long uptime = currentMillis / 1000;

    String payload = String("{\"id\":") + packetID++
                   + ",\"uptime\":" + uptime
                   + ",\"voltage\":" + voltage
                   + ",\"temp\":" + temp
                   + ",\"humidity\":" + humid + "}";

    rf95.send((uint8_t *)payload.c_str(), payload.length());
    rf95.waitPacketSent();

    Serial.println("Sent: " + payload);

    // Blink built-in LED after sending
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
