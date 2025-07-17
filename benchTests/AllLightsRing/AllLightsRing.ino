//Used to test brightness capabilities of your setup.

#include <FastLED.h>
#define LED_PIN     6
#define NUM_LEDS    60
#define INITIAL_BRIGHTNESS 220
#define MAX_BRIGHTNESS  244
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  Serial.println("Starting LED brightness test...");
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  
  // Start at working brightness
  Serial.println("Setting initial brightness to INITIAL_BRIGHTNESS");
  FastLED.setBrightness(INITIAL_BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::White);
  FastLED.show();
  delay(2000);
  
  // Slowly increase brightness
  Serial.println("Beginning gradual brightness increase...");
  for(int brightness = INITIAL_BRIGHTNESS; brightness <= MAX_BRIGHTNESS; brightness++) {
    Serial.print("Setting brightness to: ");
    Serial.println(brightness);
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(5000);
  }
  
  Serial.println("Test complete!");
}

void loop() {
  // Empty loop
}