/*
 * IoT Smart Air Purifier & Humidity Control System
 * Hardware: Arduino Uno, DHT Sensor, 6x Fans, 4x Ultrasonic Humidifiers, Pump
 * Platform: Blynk IoT (remote control + monitoring)
 * Features: Auto-threshold control + Manual override via Blynk app
 */

#define BLYNK_TEMPLATE_ID   "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "Air Purifier"
#define BLYNK_AUTH_TOKEN    "YOUR_AUTH_TOKEN"

#include <BlynkSimpleEsp32.h>
#include <DHT.h>

// ── Pin Definitions ──────────────────────────────────────────────
#define DHT_PIN        4
#define DHT_TYPE       DHT11

#define FAN_PIN        5    // Relay for 6 cooling fans
#define HUMIDIFIER_PIN 6    // Relay for 4 ultrasonic humidifiers
#define PUMP_PIN       7    // Relay for submersible pump

// ── Blynk Virtual Pins ───────────────────────────────────────────
#define VPIN_HUMIDITY     V0
#define VPIN_TEMPERATURE  V1
#define VPIN_FAN          V2   // Manual fan toggle from app
#define VPIN_HUMIDIFIER   V3   // Manual humidifier toggle from app
#define VPIN_AUTO_MODE    V4   // Auto mode toggle from app

// ── Thresholds ───────────────────────────────────────────────────
#define HUMIDITY_LOW      45   // Below this → activate humidifiers
#define HUMIDITY_HIGH     65   // Above this → stop humidifiers
#define TEMP_HIGH         30   // Above this → activate fans

// ── WiFi Credentials ─────────────────────────────────────────────
char ssid[] = "YOUR_WIFI_SSID";
char pass[] = "YOUR_WIFI_PASSWORD";

// ── State Variables ──────────────────────────────────────────────
DHT dht(DHT_PIN, DHT_TYPE);
BlynkTimer timer;

bool autoMode        = true;   // Auto mode ON by default
bool manualFan       = false;
bool manualHumid     = false;

float currentHumidity    = 0;
float currentTemperature = 0;

// ── Read & Send Sensor Data ──────────────────────────────────────
void readSensorAndControl() {
  currentHumidity    = dht.readHumidity();
  currentTemperature = dht.readTemperature();

  // Validate reading
  if (isnan(currentHumidity) || isnan(currentTemperature)) {
    Serial.println("[ERROR] DHT sensor read failed.");
    return;
  }

  Serial.print("Humidity: ");    Serial.print(currentHumidity);    Serial.print("%  ");
  Serial.print("Temperature: "); Serial.print(currentTemperature); Serial.println("°C");

  // Send live data to Blynk dashboard
  Blynk.virtualWrite(VPIN_HUMIDITY,    currentHumidity);
  Blynk.virtualWrite(VPIN_TEMPERATURE, currentTemperature);

  // ── AUTO MODE LOGIC ──────────────────────────────────────────
  if (autoMode) {

    // Humidifier control: ON below low threshold, OFF above high threshold
    if (currentHumidity < HUMIDITY_LOW) {
      digitalWrite(HUMIDIFIER_PIN, HIGH);
      digitalWrite(PUMP_PIN, HIGH);
      Serial.println("[AUTO] Humidity low → Humidifiers ON");
    } else if (currentHumidity > HUMIDITY_HIGH) {
      digitalWrite(HUMIDIFIER_PIN, LOW);
      digitalWrite(PUMP_PIN, LOW);
      Serial.println("[AUTO] Humidity OK → Humidifiers OFF");
    }

    // Fan control: ON when temperature is high
    if (currentTemperature > TEMP_HIGH) {
      digitalWrite(FAN_PIN, HIGH);
      Serial.println("[AUTO] Temp high → Fans ON");
    } else {
      digitalWrite(FAN_PIN, LOW);
      Serial.println("[AUTO] Temp OK → Fans OFF");
    }
  }
}

// ── Blynk: Auto Mode Toggle (V4) ────────────────────────────────
BLYNK_WRITE(VPIN_AUTO_MODE) {
  autoMode = param.asInt();
  Serial.print("[BLYNK] Auto mode: ");
  Serial.println(autoMode ? "ON" : "OFF");

  // If switching to manual, turn everything off first
  if (!autoMode) {
    digitalWrite(FAN_PIN,       LOW);
    digitalWrite(HUMIDIFIER_PIN, LOW);
    digitalWrite(PUMP_PIN,      LOW);
  }
}

// ── Blynk: Manual Fan Control (V2) ──────────────────────────────
BLYNK_WRITE(VPIN_FAN) {
  if (!autoMode) {
    manualFan = param.asInt();
    digitalWrite(FAN_PIN, manualFan ? HIGH : LOW);
    Serial.print("[MANUAL] Fans: ");
    Serial.println(manualFan ? "ON" : "OFF");
  }
}

// ── Blynk: Manual Humidifier Control (V3) ───────────────────────
BLYNK_WRITE(VPIN_HUMIDIFIER) {
  if (!autoMode) {
    manualHumid = param.asInt();
    digitalWrite(HUMIDIFIER_PIN, manualHumid ? HIGH : LOW);
    digitalWrite(PUMP_PIN,       manualHumid ? HIGH : LOW);
    Serial.print("[MANUAL] Humidifiers: ");
    Serial.println(manualHumid ? "ON" : "OFF");
  }
}

// ── Setup ────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(FAN_PIN,        OUTPUT);
  pinMode(HUMIDIFIER_PIN, OUTPUT);
  pinMode(PUMP_PIN,       OUTPUT);

  // Start with everything OFF
  digitalWrite(FAN_PIN,        LOW);
  digitalWrite(HUMIDIFIER_PIN, LOW);
  digitalWrite(PUMP_PIN,       LOW);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  // Read sensor and run control logic every 2 seconds
  timer.setInterval(2000L, readSensorAndControl);

  Serial.println("[SYSTEM] Air Purifier started. Auto mode: ON");
}

// ── Loop ─────────────────────────────────────────────────────────
void loop() {
  Blynk.run();
  timer.run();
}
