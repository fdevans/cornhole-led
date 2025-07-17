#include <FastLED.h>
#include <EEPROM.h>

#define DEBUG_MODE false

// LED setup - 3 separate strips
#define LEFT_STRIP_PIN     5
#define RIGHT_STRIP_PIN    6
#define RING_STRIP_PIN     7
#define LEDS_PER_SIDE      118
#define RING_LEDS          60
#define TOTAL_LEDS         296
#define BRIGHTNESS         150
#define LED_TYPE           WS2812B
#define COLOR_ORDER        GRB

// Sensor setup
#define IR_SENSOR_PIN      8
#define VIBRATION_PIN      A1  // Analog pin for vibration sensor
#define SWITCH_PIN         2   // Momentary switch for color change

// EEPROM setup
#define EEPROM_COLOR_ADDRESS 0

// LED strip arrays
CRGB leftStrip[LEDS_PER_SIDE];
CRGB rightStrip[LEDS_PER_SIDE];
CRGB ringStrip[RING_LEDS];

// Color cycling variables
int currentColorIndex = 0;
CRGB rainbowColors[] = {CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green, 
                        CRGB::Blue, CRGB::Indigo, CRGB::Purple, CRGB::Magenta};
int numColors = 8;

// Effect control variables
bool effectActive = false;
unsigned long effectStartTime = 0;
int effectType = 0; // 0 = none, 1 = vibration, 2 = hole score

// Vibration detection variables
int vibrationThreshold = 100;  // Adjust based on your sensor sensitivity
unsigned long lastVibrationTime = 0;
bool vibrationCooldown = false;

// Pattern variables for idle mode
unsigned long lastPatternUpdate = 0;
int patternStep = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Enhanced Cornhole Board LED Controller");
  
  // Initialize LED strips
  FastLED.addLeds<LED_TYPE, LEFT_STRIP_PIN, COLOR_ORDER>(leftStrip, LEDS_PER_SIDE);
  FastLED.addLeds<LED_TYPE, RIGHT_STRIP_PIN, COLOR_ORDER>(rightStrip, LEDS_PER_SIDE);
  FastLED.addLeds<LED_TYPE, RING_STRIP_PIN, COLOR_ORDER>(ringStrip, RING_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  // Setup pins
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(VIBRATION_PIN, INPUT);
  
  // Read saved color from EEPROM
  currentColorIndex = EEPROM.read(EEPROM_COLOR_ADDRESS);
  if (currentColorIndex >= numColors || currentColorIndex < 0) {
    currentColorIndex = 0;
    EEPROM.write(EEPROM_COLOR_ADDRESS, currentColorIndex);
  }
  
  Serial.print("Restored color index: ");
  Serial.println(currentColorIndex);
  
  // Initialize with idle pattern
  setIdlePattern();
  FastLED.show();
}

void setIdlePattern() {
  // Side strips: Mix of current rainbow color and white
  CRGB currentColor = rainbowColors[currentColorIndex];
  
  for (int i = 0; i < LEDS_PER_SIDE; i++) {
    // Create a mixed pattern of selected color and white
    if (i % 3 == 0) {
      leftStrip[i] = CRGB::White;
      rightStrip[i] = CRGB::White;
    } else {
      leftStrip[i] = currentColor;
      rightStrip[i] = currentColor;
    }
  }
  
  // Ring: White
  fill_solid(ringStrip, RING_LEDS, CRGB::White);
}

void vibrationStrengthMeter(int strength) {
  // Keep idle pattern as background/base
  setIdlePattern();
  
  // Calculate how many LEDs to light based on strength (0-100)
  int leftLeds = map(strength, 0, 100, 0, LEDS_PER_SIDE);
  int rightLeds = map(strength, 0, 100, 0, LEDS_PER_SIDE);
  int ringLeds = map(strength, 0, 100, 0, RING_LEDS);
  
  // Overlay red fill on top of idle pattern
  for (int i = 0; i < leftLeds; i++) {
    leftStrip[i] = CRGB::Red;
    if (i < rightLeds) rightStrip[i] = CRGB::Red;
  }
  
  // Fill ring proportionally with red
  for (int i = 0; i < ringLeds; i++) {
    ringStrip[i] = CRGB::Red;
  }
}

void sparkleParty() {
  // Random sparkles on side strips
  for (int i = 0; i < LEDS_PER_SIDE; i++) {
    if (random(10) < 3) { // 30% chance of sparkle
      CRGB sparkleColor = CRGB(random(100, 255), random(100, 255), random(100, 255));
      leftStrip[i] = sparkleColor;
      rightStrip[i] = sparkleColor;
    } else {
      leftStrip[i] = CRGB::Black;
      rightStrip[i] = CRGB::Black;
    }
  }
}

void flashGreen() {
  // Flash the ring green
  unsigned long elapsed = millis() - effectStartTime;
  bool flashOn = (elapsed / 250) % 2 == 0; // Flash every 250ms
  
  if (flashOn) {
    fill_solid(ringStrip, RING_LEDS, CRGB::Green);
  } else {
    fill_solid(ringStrip, RING_LEDS, CRGB::Black);
  }
}

int readVibrationStrength() {
  int reading = analogRead(VIBRATION_PIN);
  // Convert analog reading to strength percentage
  int strength = map(reading, 0, 1023, 0, 100);
  return constrain(strength, 0, 100);
}

void handleColorChange() {
  static bool lastButtonState = HIGH;
  static unsigned long lastButtonTime = 0;
  bool buttonState = digitalRead(SWITCH_PIN);
  
  if (buttonState != lastButtonState && (millis() - lastButtonTime > 200)) {
    if (buttonState == LOW) { // Button pressed
      currentColorIndex = (currentColorIndex + 1) % numColors;
      EEPROM.write(EEPROM_COLOR_ADDRESS, currentColorIndex);
      
      Serial.print("Color changed to index: ");
      Serial.println(currentColorIndex);
      
      // Update idle pattern if no effect is active
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
    if (millis() - lastVibrationTime > 1000) { // 1 second cooldown
      vibrationCooldown = false;
    }
    return;
  }
  
  int vibrationStrength = readVibrationStrength();
  
  if (vibrationStrength > vibrationThreshold) {
    Serial.print("Vibration detected! Strength: ");
    Serial.println(vibrationStrength);
    
    effectActive = true;
    effectType = 1; // Vibration effect
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
  
  // Debounce
  if (sensorState != lastSensorState) {
    lastChangeTime = millis();
  }
  
  if ((millis() - lastChangeTime) > 100) {
    if (sensorState == LOW && !beamBroken) { // Beam broken
      Serial.println("HOLE SCORED!");
      effectActive = true;
      effectType = 2; // Hole score effect
      effectStartTime = millis();
      beamBroken = true;
    } else if (sensorState == HIGH && beamBroken) { // Beam restored
      beamBroken = false;
    }
  }
  
  lastSensorState = sensorState;
}

void updateEffects() {
  if (!effectActive) return;
  
  unsigned long elapsed = millis() - effectStartTime;
  
  if (effectType == 1) { // Vibration effect
    if (elapsed < 2000) { // Show for 2 seconds
      // Vibration effect already set, just maintain it
    } else {
      // Return to idle
      effectActive = false;
      setIdlePattern();
    }
  } else if (effectType == 2) { // Hole score effect
    if (elapsed < 3000) { // Show for 3 seconds
      sparkleParty();
      flashGreen();
    } else {
      // Return to idle
      effectActive = false;
      setIdlePattern();
    }
  }
}

void updateIdlePattern() {
  if (effectActive) return;
  
  // Add subtle breathing effect to idle pattern
  if (millis() - lastPatternUpdate > 50) {
    patternStep++;
    if (patternStep >= 360) patternStep = 0;
    
    // Subtle brightness variation
    float breathe = sin(patternStep * 0.01745) * 0.3 + 0.7; // 0.4 to 1.0
    int brightness = BRIGHTNESS * breathe;
    FastLED.setBrightness(brightness);
    
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
  delay(10);
}