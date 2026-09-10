
#include <Wire.h>
#include <SparkFun_AS726X.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <LoRa.h>


#define MOX1_PIN 32   // MQ135 - general air quality
#define MOX2_PIN 33   // MQ138 - organic solvents
#define MOX3_PIN 34   // MQ137 - ammonia
#define MOX4_PIN 35   // MQ3   - alcohol vapors

#define BUZZER_PIN 25
#define LED_PIN    26

#define LORA_SS   5
#define LORA_RST  14
#define LORA_DIO0 2
#define LORA_FREQUENCY 866E6

#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_RESET  -1


AS726X spectralSensor;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);


struct SensorReading {
  float mox[4];        // MOX1..MOX4 raw analog values (0-4095)
  float spectral[6];   // AS7263 channels: R,S,T,U,V,W (610-860nm)
};

struct ReferenceEntry {
  const char* name;
  SensorReading pattern;
};


SensorReading baseline;


ReferenceEntry referenceLibrary[] = {
  { "Flour",       { {600, 400, 450, 500}, {200, 230, 210, 260, 240, 220} } },
  { "Baking Soda", { {550, 420, 500, 480}, {180, 260, 240, 230, 250, 210} } },
  { "Turmeric",    { {650, 450, 470, 510}, {300, 190, 260, 220, 200, 280} } },
};
const int NUM_REFERENCES = sizeof(referenceLibrary) / sizeof(ReferenceEntry);

// ---------- THRESHOLDS — TUNE THESE AFTER REAL-WORLD TESTING ----------
const float MOX_DEVIATION_THRESHOLD      = 80.0;   // raw ADC units from baseline
const float SPECTRAL_DEVIATION_THRESHOLD = 40.0;   // raw sensor units from baseline
const float MATCH_DISTANCE_THRESHOLD     = 60.0;   // "close enough" to call it a match

// ---------- ALERT STATE (used to avoid spamming LoRa) ----------
enum AlertLevel { GREEN, YELLOW, RED };
AlertLevel lastSentAlert = GREEN;


void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // --- OLED init ---
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Booting...");
  display.display();

  // --- AS7263 init ---
  if (!spectralSensor.begin()) {
    Serial.println("AS7263 not detected. Check wiring.");
  }

  // --- LoRa init ---
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("LoRa init failed. Check wiring/frequency.");
  }

  // --- Baseline calibration ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Calibrating baseline...");
  display.println("Hold in clean air");
  display.display();

  captureBaseline();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Ready.");
  display.display();
  delay(1000);
}


void loop() {
  SensorReading current = takeReading();

  float moxDeviation = computeMoxDeviation(current, baseline);
  float spectralDeviation = computeSpectralDeviation(current, baseline);

  AlertLevel level = GREEN;
  if (moxDeviation > MOX_DEVIATION_THRESHOLD && spectralDeviation > SPECTRAL_DEVIATION_THRESHOLD) {
    level = RED;
  } else if (moxDeviation > MOX_DEVIATION_THRESHOLD || spectralDeviation > SPECTRAL_DEVIATION_THRESHOLD) {
    level = YELLOW;
  }

  const char* matchName = "Unknown";
  if (level != GREEN) {
    matchName = findClosestMatch(current);
  }

  updateDisplay(level, matchName, moxDeviation, spectralDeviation);
  updateAlertOutputs(level);

  if (level != lastSentAlert) {
    sendLoRaAlert(level, matchName);
    lastSentAlert = level;
  }

  delay(1000); // sample once per second
}


// Capture a "clean/neutral" baseline by averaging readings over 5 seconds
void captureBaseline() {
  SensorReading sum = {};
  const int samples = 5;

  for (int i = 0; i < samples; i++) {
    SensorReading r = takeReading();
    for (int m = 0; m < 4; m++) sum.mox[m] += r.mox[m];
    for (int s = 0; s < 6; s++) sum.spectral[s] += r.spectral[s];
    delay(1000);
  }

  for (int m = 0; m < 4; m++) baseline.mox[m] = sum.mox[m] / samples;
  for (int s = 0; s < 6; s++) baseline.spectral[s] = sum.spectral[s] / samples;
}

// Read both sensor arrays and return a combined reading
SensorReading takeReading() {
  SensorReading r;

  r.mox[0] = analogRead(MOX1_PIN);
  r.mox[1] = analogRead(MOX2_PIN);
  r.mox[2] = analogRead(MOX3_PIN);
  r.mox[3] = analogRead(MOX4_PIN);

  spectralSensor.takeMeasurements();
  r.spectral[0] = spectralSensor.getCalibratedR();
  r.spectral[1] = spectralSensor.getCalibratedS();
  r.spectral[2] = spectralSensor.getCalibratedT();
  r.spectral[3] = spectralSensor.getCalibratedU();
  r.spectral[4] = spectralSensor.getCalibratedV();
  r.spectral[5] = spectralSensor.getCalibratedW();

  return r;
}

// Euclidean-style deviation of MOX readings from baseline
float computeMoxDeviation(SensorReading &current, SensorReading &base) {
  float sumSq = 0;
  for (int i = 0; i < 4; i++) {
    float diff = current.mox[i] - base.mox[i];
    sumSq += diff * diff;
  }
  return sqrt(sumSq / 4.0);
}

// Euclidean-style deviation of spectral readings from baseline
float computeSpectralDeviation(SensorReading &current, SensorReading &base) {
  float sumSq = 0;
  for (int i = 0; i < 6; i++) {
    float diff = current.spectral[i] - base.spectral[i];
    sumSq += diff * diff;
  }
  return sqrt(sumSq / 6.0);
}

// Distance between a live reading and one reference entry (mox + spectral combined)
float distanceToReference(SensorReading &current, SensorReading &ref) {
  float sumSq = 0;
  for (int i = 0; i < 4; i++) {
    float diff = current.mox[i] - ref.mox[i];
    sumSq += diff * diff;
  }
  for (int i = 0; i < 6; i++) {
    float diff = current.spectral[i] - ref.spectral[i];
    sumSq += diff * diff;
  }
  return sqrt(sumSq / 10.0);
}

// Find the closest matching entry in the reference library, or "Unknown"
const char* findClosestMatch(SensorReading &current) {
  float bestDistance = 1e9;
  const char* bestName = "Unknown";

  for (int i = 0; i < NUM_REFERENCES; i++) {
    float d = distanceToReference(current, referenceLibrary[i].pattern);
    if (d < bestDistance) {
      bestDistance = d;
      bestName = referenceLibrary[i].name;
    }
  }

  if (bestDistance > MATCH_DISTANCE_THRESHOLD) {
    return "Unknown";
  }
  return bestName;
}

// Update OLED with current status
void updateDisplay(AlertLevel level, const char* matchName, float moxDev, float specDev) {
  display.clearDisplay();
  display.setCursor(0, 0);

  switch (level) {
    case GREEN:  display.println("STATUS: NORMAL"); break;
    case YELLOW: display.println("STATUS: CAUTION"); break;
    case RED:    display.println("STATUS: ALERT!"); break;
  }

  display.print("MOX dev: ");
  display.println(moxDev);
  display.print("Spec dev: ");
  display.println(specDev);

  if (level != GREEN) {
    display.print("Match: ");
    display.println(matchName);
  }

  display.display();
}

// Drive buzzer/LED based on alert level
void updateAlertOutputs(AlertLevel level) {
  if (level == RED) {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 2000, 300); // 300ms beep
  } else if (level == YELLOW) {
    digitalWrite(LED_PIN, HIGH);
    noTone(BUZZER_PIN);
  } else {
    digitalWrite(LED_PIN, LOW);
    noTone(BUZZER_PIN);
  }
}

// Send an alert packet over LoRa to the base station
void sendLoRaAlert(AlertLevel level, const char* matchName) {
  const char* levelStr = (level == RED) ? "RED" : (level == YELLOW) ? "YELLOW" : "GREEN";

  LoRa.beginPacket();
  LoRa.print("ALERT,");
  LoRa.print(levelStr);
  LoRa.print(",");
  LoRa.print(matchName);
  LoRa.print(",");
  LoRa.print(millis());
  LoRa.endPacket();
}