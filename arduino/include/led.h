#include <Arduino.h>

// LED Control
# define LED_Enable A0
# define LED_0 A1
# define LED_1 A2
# define LED_2 A3
# define LED_Disable A4

// LED IDs
# define LED_ERR_ID 1

// Misc Settings
#define LED_RAINBOW_PERIOD 500

// Functions
void setupLEDs();
void setLed(byte id);
void LEDRainbow();