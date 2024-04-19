#include "led.h"

void setupLEDs() {
    pinMode(LED_Enable, OUTPUT);
    pinMode(LED_0, OUTPUT);
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_Disable, OUTPUT);
    digitalWrite(LED_Disable, LOW);
}

void setLed(byte id) {
    id -= 1;
    digitalWrite(LED_Enable, LOW);
    digitalWrite(LED_0, LOW);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    if (id >= 0 && id < 8) {
        digitalWrite(LED_Enable, LOW);
        digitalWrite(LED_0, id & 0b001);
        digitalWrite(LED_1, id & 0b010);
        digitalWrite(LED_2, id & 0b100);
        digitalWrite(LED_Enable, HIGH);
    }
}

byte led_rainbow_state = 0;
int led_rainbow_prev_state_time = 0;
void LEDRainbow() {
  int currTime = millis();
  if (currTime - led_rainbow_prev_state_time > LED_RAINBOW_PERIOD) {
    led_rainbow_prev_state_time = currTime;
    setLed(led_rainbow_state+1);
    led_rainbow_state = (led_rainbow_state+1)%8;
  }
}