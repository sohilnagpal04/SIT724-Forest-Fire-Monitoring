#include <SPI.h>
#include <RH_RF95.h>
#include <DHT.h>

// -------------------- Pin Config --------------------
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define DHTPIN 5
#define DHTTYPE DHT22
#define VOLTAGE_PIN A0

#define RF95_FREQ 915.0
#define TX_POWER 14
#define SPREADING_FACTOR 7

// -------------------- Libraries and Objects --------------------
RH_RF95 rf95(RFM95_CS, RFM95_INT);
DHT dht(DHTPIN, DHTTYPE);

// -------------------- Timing --------------------
unsigned long lastTransmit = 0;
const unsigned long transmitInterval = 15000;
unsigned long startTime;
int packetID = 0;

// -------------------- Setup --------------------
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
  Serial.println("LoRa initialized");

  dht.begin();
  delay(2000);  // Let DHT stabilize

  pinMode(LED_BUILTIN, OUTPUT);

  startTime = millis();
}

// -------------------- Voltage Divider Read --------------------
float readVoltage() {
  int raw = analogRead(VOLTAGE_PIN);
  float measuredV = (raw * 3.3) / 1023.0; // ADC = 3.3V ref
  float actualV = measuredV * (10.0 + 5.0) / 10.0; // Divider: 10k & 5k
  return actualV;
}

// -------------------- Main Loop --------------------
void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastTransmit >= transmitInterval) {
    lastTransmit = currentMillis;

    float temp = dht.readTemperature();
    float humid = dht.readHumidity();

    if (isnan(temp) || isnan(humid)) {
      Serial.println("⚠️ DHT22 data invalid, skipping this cycle.");
      return;
    }

    float voltage = readVoltage();
    unsigned long uptime = currentMillis / 1000;

    String payload = String("{\"id\":") + packetID++
                   + ",\"uptime\":" + uptime
                   + ",\"voltage\":" + voltage
                   + ",\"temp\":" + temp
                   + ",\"humidity\":" + humid + "}";

    rf95.send((uint8_t *)payload.c_str(), payload.length());
    rf95.waitPacketSent();

    Serial.println("📡 Sent: " + payload);

    // Blink built-in LED to indicate transmission
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
