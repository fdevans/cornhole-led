# Cornhole LED Board Controller

An Arduino-based LED controller for cornhole boards featuring dynamic lighting effects, vibration sensing, and hole detection.

## Features

- **Dynamic LED Effects**: 296 WS2812B LEDs across 3 strips (2 side strips of 118 LEDs each + 60 LED ring)
- **Vibration Detection**: Strength meter that fills with red based on impact intensity
- **Hole Scoring**: Sparkle party effect on sides with flashing green ring when bag goes through hole
- **Color Cycling**: 8 rainbow colors selectable via momentary switch
- **Idle Pattern**: Mixed rainbow color and white pattern with subtle breathing effect
- **Persistent Settings**: Color selection saved to EEPROM

## Hardware Requirements

### Components
- Arduino Uno R3
- 3x WS2812B LED strips (total 296 LEDs)
- IR Break Beam Sensor (Adafruit ADA2168)
- Vibration Sensor Module (analog output)
- Momentary push button switch
- 11.1V 6000mAh Li-ion battery pack
- Buck converter (5.3V-32V to 1.2V-32V, 12A)
- Power switch

### Wiring
```
LED Strips:
- Left side strip (118 LEDs)  → Pin 5
- Right side strip (118 LEDs) → Pin 6  
- Ring strip (60 LEDs)        → Pin 7

Sensors:
- IR Break Beam Sensor        → Pin 8
- Vibration Sensor (analog)   → Pin A1
- Color Change Button         → Pin 2

Power:
- Battery → Arduino VIN (via switch)
- Battery → Buck Converter → 5V LED Power
```

## Installation

1. Clone this repository
2. Install required libraries:
   ```
   - FastLED
   - EEPROM (included with Arduino IDE)
   ```
3. Upload the code to your Arduino Uno
4. Wire components according to the wiring diagram
5. Adjust `vibrationThreshold` and `BRIGHTNESS` values as needed

## Usage

### Color Selection
- Press the momentary button to cycle through 8 rainbow colors
- Selected color is automatically saved to EEPROM
- Color persists between power cycles

### Effects

**Idle Mode**: 
- Side strips display mixed pattern of selected color and white
- Ring displays solid white
- Subtle breathing brightness effect

**Vibration Effect**:
- Red strength meter fills based on impact intensity
- Overlays on top of idle pattern
- Displays for 2 seconds then returns to idle

**Hole Score Effect**:
- Side strips show random colorful sparkles
- Ring flashes green for 3 seconds
- Returns to idle mode automatically

## Configuration

### Adjustable Parameters
```cpp
#define BRIGHTNESS 150          // LED brightness (0-255)
int vibrationThreshold = 100;   // Vibration sensitivity
```

### Color Customization
Modify the `rainbowColors` array to change available colors:
```cpp
CRGB rainbowColors[] = {CRGB::Red, CRGB::Orange, CRGB::Yellow, 
                        CRGB::Green, CRGB::Blue, CRGB::Indigo, 
                        CRGB::Purple, CRGB::Magenta};
```

## Power Consumption

- Idle mode: ~15W (mixed color/white pattern)
- Full brightness: ~18W per meter (theoretical max)
- Battery life: ~4-6 hours continuous use

## Troubleshooting

### Common Issues

**LEDs not lighting up**:
- Check 5V power supply to LED strips
- Verify data pin connections
- Ensure proper grounding

**Vibration sensor too sensitive**:
- Increase `vibrationThreshold` value
- Check sensor mounting and isolation

**IR sensor false triggers**:
- Ensure proper alignment of IR transmitter/receiver
- Check for ambient light interference

**Colors not saving**:
- Verify EEPROM functionality
- Check button wiring and debouncing

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- FastLED library for WS2812B control
- Arduino community for sensor libraries and examples
- Cornhole community for inspiration and feedback