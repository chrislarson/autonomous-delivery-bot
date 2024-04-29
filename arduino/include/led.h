#include <Arduino.h>

// LED Control
# define LED_Enable A0
# define LED_0 A1
# define LED_1 A2
# define LED_2 A3
# define LED_Disable A4

// LED IDs
# define LED_ERR_ID 1
# define LED_READY_ID 2
# define LED_DRIVE_ID 3
# define LED_PWM_ID 4
# define LED_FIND_TARGETS_ID 5 // Jetson sets this
# define LED_VELOCITY_ID 6
# define LED_TURN_ID 7
# define LED_WAIT_ID 8

// Misc Settings
#define LED_RAINBOW_PERIOD 500

// Functions
void setupLEDs();
void setLed(byte id);
void LEDRainbow();