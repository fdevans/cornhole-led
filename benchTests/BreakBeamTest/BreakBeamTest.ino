#include <../libraries/FastLED.h>

#define DEBUG_MODE false

// LED setup
#define LED_PIN     6
#define NUM_LEDS    60
#define BRIGHTNESS  150
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB


// IR Break Beam setup
#define IR_SENSOR_PIN 5

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);
  Serial.println("IR Break Beam Sensor Test with Auto-Reset");
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(IR_SENSOR_PIN, INPUT_PULLUP);
  
  // Start with green ring
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
}

void loop() {
  int sensorState = digitalRead(IR_SENSOR_PIN);
  
  // Show real-time readings
  if (DEBUG_MODE == true) {
    Serial.print("Raw sensor reading: ");
    Serial.println(sensorState);
  }
  // Debounce variables
  static int lastStableState = 1;  // Assume beam starts intact
  static unsigned long lastChangeTime = 0;
  static int lastReading = 1;
  
  // Auto-reset variables
  static unsigned long beamBrokenTime = 0;
  static bool autoResetActive = false;
  
  // Debounce logic
  if (sensorState != lastReading) {
    lastChangeTime = millis();
    Serial.println("State change detected, starting debounce...");
  }
  
  if ((millis() - lastChangeTime) > 100) {  // 100ms debounce
    if (sensorState != lastStableState) {
      lastStableState = sensorState;
      
      if (sensorState == 0) {  // Beam broken
        Serial.println("BEAM BROKEN (stable) - RED");
        fill_solid(leds, NUM_LEDS, CRGB::Red);
        FastLED.show();
        beamBrokenTime = millis();
        autoResetActive = true;
      } else {  // Beam intact
        Serial.println("BEAM INTACT (stable) - GREEN");
        fill_solid(leds, NUM_LEDS, CRGB::Green);
        FastLED.show();
        autoResetActive = false;
      }
    }
  }
  
  // Auto-reset after 2 seconds
  if (autoResetActive && (millis() - beamBrokenTime > 2000)) {
    Serial.println("AUTO-RESET to green after 2 seconds");
    fill_solid(leds, NUM_LEDS, CRGB::Green);
    FastLED.show();
    autoResetActive = false;
    lastStableState = 1;
  }
  
  lastReading = sensorState;
  delay(200);  // Slower for easier reading
}