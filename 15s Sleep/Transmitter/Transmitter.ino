#include <SPI.h>
#include <RH_RF95.h>
#include <DHT.h>
#include <ArduinoLowPower.h>

// ---------- Pin Definitions ----------
#define RFM95_CS     8
#define RFM95_RST    4
#define RFM95_INT    3
#define DHTPIN       5
#define DHTTYPE      DHT22
#define VOLTAGE_PIN  A0

// ---------- LoRa Settings ----------
#define RF95_FREQ         915.0
#define TX_POWER          14
#define SPREADING_FACTOR  7

// ---------- Globals ----------
DHT dht(DHTPIN, DHTTYPE);
RH_RF95 rf95(RFM95_CS, RFM95_INT);
int packetID = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);  // USB stability delay

  // LoRa module reset
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH); delay(10);
  digitalWrite(RFM95_RST, LOW);  delay(10);
  digitalWrite(RFM95_RST, HIGH); delay(10);

  if (!rf95.init()) {
    Serial.println("LoRa init failed");
    while (1);
  }

  rf95.setFrequency(RF95_FREQ);
  rf95.setTxPower(TX_POWER, false);
  rf95.setSpreadingFactor(SPREADING_FACTOR);
  Serial.println("LoRa transmitter ready");

  dht.begin();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  unsigned long activeStart = millis();  // Start timing uptime

  delay(2000);  // DHT warm-up every cycle
  float temp = dht.readTemperature();
  float humid = dht.readHumidity();

  if (isnan(temp) || isnan(humid)) {
    Serial.println("⚠️ Skipping: DHT returned NaN");
    delay(1000);
    LowPower.deepSleep(15000);
    return;
  }

  int raw = analogRead(VOLTAGE_PIN);
  float voltage = (raw * 3.3 / 1023.0) * (10.0 + 5.0) / 10.0;

  unsigned long activeEnd = millis();
  float cycleUptime = (activeEnd - activeStart) / 1000.0;

  // Create payload
  String payload = String("{\"id\":") + packetID++
                 + ",\"uptime\":" + String(cycleUptime, 2)
                 + ",\"voltage\":" + voltage
                 + ",\"temp\":" + temp
                 + ",\"humidity\":" + humid + "}";

  // Send
  rf95.send((uint8_t *)payload.c_str(), payload.length());
  rf95.waitPacketSent();
  Serial.println("📡 Sent: " + payload);

  // Blink LED
  digitalWrite(LED_BUILTIN, HIGH); delay(100);
  digitalWrite(LED_BUILTIN, LOW);

  // Pad wake time to 1s if needed
  unsigned long totalAwake = millis() - activeStart;
  if (totalAwake < 1000) {
    delay(1000 - totalAwake);
  }

  // 💤 Sleep for 15 seconds
  LowPower.deepSleep(15000);
}
