#include <FastLED.h>
#include <EEPROM.h>

// LED setup
#define LED_PIN     6
#define NUM_LEDS    60
#define BRIGHTNESS  150
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

// Momentary switch setup
#define SWITCH_PIN 7

// EEPROM setup
#define EEPROM_COLOR_ADDRESS 0  // Memory location to store color

CRGB leds[NUM_LEDS];

// Color cycling variables
int currentColor = 0;
CRGB colors[] = {CRGB::Green, CRGB::Blue, CRGB::Purple, CRGB::Orange, CRGB::White};
int numColors = 5;

void setup() {
  Serial.begin(9600);
  Serial.println("LED Color Changer Test");
  
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  
  // Read saved color from EEPROM
  currentColor = EEPROM.read(EEPROM_COLOR_ADDRESS);
  
  // Validate the saved color (in case EEPROM was corrupted)
  if (currentColor >= numColors || currentColor < 0) {
    currentColor = 0;  // Default to first color if invalid
    EEPROM.write(EEPROM_COLOR_ADDRESS, currentColor);
  }
  
  Serial.print("Restored color from EEPROM: ");
  Serial.println(currentColor);
  
  // Start with saved color
  fill_solid(leds, NUM_LEDS, colors[currentColor]);
  FastLED.show();
}

void loop() {
  // Button handling with debounce
  static bool lastButtonState = HIGH;
  static unsigned long lastButtonTime = 0;
  bool buttonState = digitalRead(SWITCH_PIN);
  
  if (buttonState != lastButtonState && (millis() - lastButtonTime > 50)) {
    if (buttonState == LOW) {  // Button pressed
      currentColor = (currentColor + 1) % numColors;
      
      // Save new color to EEPROM
      EEPROM.write(EEPROM_COLOR_ADDRESS, currentColor);
      
      Serial.print("Color changed to: ");
      Serial.print(currentColor);
      Serial.println(" (saved to EEPROM)");
      
      // Update LEDs with new color
      fill_solid(leds, NUM_LEDS, colors[currentColor]);
      FastLED.show();
    }
    lastButtonTime = millis();
  }
  lastButtonState = buttonState;
  
  delay(10);
}