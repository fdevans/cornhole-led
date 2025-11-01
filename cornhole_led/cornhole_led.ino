#include <FastLED.h>
#include <EEPROM.h>

#define DEBUG_MODE false

// LED setup - 3 separate strips
#define LEFT_STRIP_PIN 5
#define RIGHT_STRIP_PIN 6
#define RING_STRIP_PIN 7
#define LEDS_PER_SIDE 118
#define RING_LEDS 60
#define TOTAL_LEDS 296
#define BRIGHTNESS 150
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

// Sensor setup
#define IR_SENSOR_PIN 8
#define VIBRATION_PIN A1
#define SWITCH_PIN 2

// EEPROM setup
#define EEPROM_COLOR_ADDRESS 0

// LED strip arrays
CRGB leftStrip[LEDS_PER_SIDE];
CRGB rightStrip[LEDS_PER_SIDE];
CRGB ringStrip[RING_LEDS];

// Color cycling variables
int currentColorIndex = 0;
CRGB rainbowColors[] = { CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green,
                         CRGB::Blue, CRGB::Indigo, CRGB::Purple, CRGB::Magenta };

int numColors = 8;                    // solid colors
int rainbowModeIndex = 8;             // Index 8 = rainbow mode
int goldBlueModeIndex = 9;            // Index 9 = gold to dark blue pattern
int totalColorOptions = 10;           // 8 solid colors + 1 rainbow mode + 1 gold/blue pattern

// Define your custom colors
CRGB goldColor = CRGB(255, 215, 0);     // Gold
CRGB darkBlue = CRGB(0, 0, 139);        // Dark Blue
CRGB redColor = CRGB::Red;              // Red

// Effect control variables
bool effectActive = false;
unsigned long effectStartTime = 0;
int effectType = 0;  // 0 = none, 1 = vibration, 2 = hole score

// Vibration detection variables
int vibrationThreshold = 4;
unsigned long lastVibrationTime = 0;
bool vibrationCooldown = false;

// Pattern variables for idle mode
unsigned long lastPatternUpdate = 0;
int patternStep = 0;

int maxRecordedStrength = 1;  // Avoid div/0

void setup() {
  Serial.begin(9600);
  Serial.println("Enhanced Cornhole Board LED Controller");

  FastLED.addLeds<LED_TYPE, LEFT_STRIP_PIN, COLOR_ORDER>(leftStrip, LEDS_PER_SIDE);
  FastLED.addLeds<LED_TYPE, RIGHT_STRIP_PIN, COLOR_ORDER>(rightStrip, LEDS_PER_SIDE);
  FastLED.addLeds<LED_TYPE, RING_STRIP_PIN, COLOR_ORDER>(ringStrip, RING_LEDS);
  FastLED.setBrightness(BRIGHTNESS);

  FastLED.clear(true);  // Prevent power-on LED flash

  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(VIBRATION_PIN, INPUT);

  currentColorIndex = EEPROM.read(EEPROM_COLOR_ADDRESS);
  if (currentColorIndex >= totalColorOptions || currentColorIndex < 0) {
    currentColorIndex = 0;
    EEPROM.write(EEPROM_COLOR_ADDRESS, currentColorIndex);
  }

  Serial.print("Restored color index: ");
  Serial.println(currentColorIndex);
  if (currentColorIndex == rainbowModeIndex) {
    Serial.println("Rainbow mode active!");
  }

  setIdlePattern();
  FastLED.show();
}

void setIdlePattern() {
  if (currentColorIndex == rainbowModeIndex) {
    // Animated rainbow with white accents
    static uint8_t hueBase = 0;
    hueBase += 3;  // Adjust speed of rainbow here

    for (int i = 0; i < LEDS_PER_SIDE; i++) {
      if (i % 3 == 0) {
        leftStrip[i] = CRGB::White;
        rightStrip[i] = CRGB::White;
      } else {
        leftStrip[i] = CHSV(hueBase + i * 2, 255, 255);
        rightStrip[i] = CHSV(hueBase + i * 2, 255, 255);
      }
    }
  } else if (currentColorIndex == goldBlueModeIndex) {
    // Gold to Dark Blue pattern
    for (int i = 0; i < LEDS_PER_SIDE; i++) {
      if (i < 42) {
        // First 42 LEDs: fade from Gold to Dark Blue
        float fadeProgress = (float)i / 41.0;  // 0.0 to 1.0
        leftStrip[i] = blend(goldColor, darkBlue, fadeProgress * 255);
        rightStrip[i] = blend(goldColor, darkBlue, fadeProgress * 255);
      } else if (i >= 99) {  // Last 16 LEDs (116 - 16 = 100)
        // Last 16 LEDs: Dark Blue
        leftStrip[i] = darkBlue;
        rightStrip[i] = darkBlue;
      } else {
        // Everything in between (LEDs 42-101): Red
        leftStrip[i] = redColor;
        rightStrip[i] = redColor;
      }
    }
  } else {
    // Normal solid color pattern
    CRGB currentColor = rainbowColors[currentColorIndex];
    for (int i = 0; i < LEDS_PER_SIDE; i++) {
      if (i % 3 == 0) {
        leftStrip[i] = CRGB::White;
        rightStrip[i] = CRGB::White;
      } else {
        leftStrip[i] = currentColor;
        rightStrip[i] = currentColor;
      }
    }
  }

  fill_solid(ringStrip, RING_LEDS, CRGB::White);
}


CRGB getGradientColor(float p) {
  p = constrain(p, 0.0, 1.0);
  if (p < 0.5) {
    return blend(CRGB::Green, CRGB::Yellow, p * 2 * 255);
  } else {
    return blend(CRGB::Yellow, CRGB::Red, (p - 0.5) * 2 * 255);
  }
}

void vibrationStrengthMeter(int strength) {
  setIdlePattern();

  if (strength > maxRecordedStrength) {
    maxRecordedStrength = strength;
    Serial.print("🎯 New Max Strength: ");
    Serial.println(maxRecordedStrength);
  }

  int usableMax = constrain(maxRecordedStrength, 1, 1023);
  const float exponent = 1.7;
  float scaled = pow((float)strength / usableMax, exponent);
  scaled = constrain(scaled, 0.0, 1.0);

  int targetRingLeds = scaled * (RING_LEDS / 2);
  const int animationSteps = targetRingLeds;
  const int animationDelay = 15;

  for (int i = 0; i <= animationSteps; i++) {
    int ledOffset = i;
    float verticalProgress = (float)ledOffset / (RING_LEDS / 2);
    CRGB color = getGradientColor(verticalProgress);

    int bottomLeft = ledOffset;
    int bottomRight = RING_LEDS - 1 - ledOffset;

    if (bottomLeft < RING_LEDS) ringStrip[bottomLeft] = color;
    if (bottomRight >= 0)       ringStrip[bottomRight] = color;

    FastLED.show();
    delay(animationDelay);
  }
}

void sparkleParty() {
  for (int i = 0; i < LEDS_PER_SIDE; i++) {
    if (random(10) < 2) {  // Only 20% chance to turn off (was 30% before)
      leftStrip[i] = CRGB::Black;
      rightStrip[i] = CRGB::Black;
    } else {
      // 80% of LEDs get bright, vibrant random colors
      CRGB sparkleColor = CRGB(random(150, 255), random(150, 255), random(150, 255));
      leftStrip[i] = sparkleColor;
      rightStrip[i] = sparkleColor;
    }
  }
}

void flashGreen() {
  unsigned long elapsed = millis() - effectStartTime;
  bool flashOn = (elapsed / 150) % 2 == 0;  // Faster flash (was 250)

  fill_solid(ringStrip, RING_LEDS, flashOn ? CRGB::Green : CRGB::Black);
}

int readVibrationStrength() {
  long total = 0;
  const int samples = 10;
  for (int i = 0; i < samples; i++) {
    total += analogRead(VIBRATION_PIN);
    delay(2);
  }
  int avg = total / samples;
  int strength = map(avg, 0, 1023, 0, 100);
  return constrain(strength, 0, 100);
}

void handleColorChange() {
  static bool lastButtonState = HIGH;
  static unsigned long lastButtonTime = 0;
  bool buttonState = digitalRead(SWITCH_PIN);

  if (buttonState != lastButtonState && (millis() - lastButtonTime > 200)) {
    if (buttonState == LOW) {
      currentColorIndex = (currentColorIndex + 1) % totalColorOptions;

      if (EEPROM.read(EEPROM_COLOR_ADDRESS) != currentColorIndex) {
        EEPROM.write(EEPROM_COLOR_ADDRESS, currentColorIndex);
      }

      Serial.print("Color changed to index: ");
      Serial.println(currentColorIndex);
      if (currentColorIndex == rainbowModeIndex) {
        Serial.println("Rainbow mode activated!");
      } else if (currentColorIndex == goldBlueModeIndex) {
        Serial.println("Gold to Dark Blue pattern activated!");
      }

      if (!effectActive) {
        setIdlePattern();
      }
    }
    lastButtonTime = millis();
  }
  lastButtonState = buttonState;
}

void handleVibration() {
  if (vibrationCooldown) {
    if (millis() - lastVibrationTime > 1000) {
      vibrationCooldown = false;
    }
    return;
  }

  int vibrationStrength = readVibrationStrength();

  if (vibrationStrength > vibrationThreshold) {
    Serial.print("Vibration detected! Strength: ");
    Serial.println(vibrationStrength);

    effectActive = true;
    effectType = 1;
    effectStartTime = millis();
    lastVibrationTime = millis();
    vibrationCooldown = true;

    vibrationStrengthMeter(vibrationStrength);
  }
}

void handleIRSensor() {
  static bool lastSensorState = HIGH;
  static unsigned long lastChangeTime = 0;
  static bool beamBroken = false;

  bool sensorState = digitalRead(IR_SENSOR_PIN);

  if (sensorState != lastSensorState) {
    lastChangeTime = millis();
  }

  if ((millis() - lastChangeTime) > 10) {
    if (sensorState == LOW && !beamBroken) {
      Serial.println("HOLE SCORED!");
      effectActive = true;
      effectType = 2;
      effectStartTime = millis();
      beamBroken = true;
    } else if (sensorState == HIGH && beamBroken) {
      beamBroken = false;
    }
  }

  lastSensorState = sensorState;
}

void updateEffects() {
  if (!effectActive) return;

  unsigned long elapsed = millis() - effectStartTime;

  if (effectType == 1) {
    if (elapsed >= 2000) {
      effectActive = false;
      setIdlePattern();
    }
  } else if (effectType == 2) {
    if (elapsed < 3000) {
      sparkleParty();
      flashGreen();
    } else {
      effectActive = false;
      setIdlePattern();
    }
  }
}

void updateIdlePattern() {
  if (effectActive) return;

  if (millis() - lastPatternUpdate > 50) {
    patternStep++;
    if (patternStep >= 360) patternStep = 0;

    float breathe = sin(patternStep * 0.01745) * 0.3 + 0.7;
    int brightness = BRIGHTNESS * breathe;
    FastLED.setBrightness(brightness);

    // Refresh rainbow animation
    if (currentColorIndex == rainbowModeIndex) {
      setIdlePattern();  // Force refresh rainbow hues
    }

    lastPatternUpdate = millis();
  }
}


void loop() {
  handleColorChange();
  handleVibration();
  handleIRSensor();
  updateEffects();
  updateIdlePattern();

  FastLED.show();
  
  // Faster updates during sparkle party effect
  if (effectActive && effectType == 2) {
    delay(25);  // Much faster sparkle during hole score celebration
  } else {
    delay(10);  // Normal speed for everything else
  }
}