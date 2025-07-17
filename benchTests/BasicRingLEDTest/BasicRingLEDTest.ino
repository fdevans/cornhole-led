#include <FastLED.h>

#define LED_PIN     6       // Pin where the LED ring is connected
#define NUM_LEDS    60      // Number of LEDs in the ring
#define BRIGHTNESS  100      // Adjust 0–255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
}

void loop() {
  // Light each LED red, one at a time
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
    FastLED.show();
    delay(50);
  }

  // Turn each LED off, one at a time
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(25);
  }
}
