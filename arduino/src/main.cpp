#include <Arduino.h>

#include "serial.h"
#include "commands.h"
#include "led.h"
#include "motors.h"
#include "skid_steer.h"

void periodic();

//============

void setup() {
  // Setup Robot Subsystems
  setupSerial();
  setupLEDs();
  setupMotors();
  Initialize_Skid_Steer(getLeftEncoderCounts(), getRightEncoderCounts());
}

//============

void loop() {
  while (Serial.available()) {
    if (updateSerial()) {
      execCmd(nextCmdType());
    } else {
      break;
    }
  }

  if (isEnabled()) {
    periodic();
  } else {
    LEDRainbow();
  }
}

unsigned long update_prev_time = millis();
void periodic() {
  //updateEncoders();
  sendSysID();
  Skid_Steer_Update(getLeftEncoderCounts(), getRightEncoderCounts(), update_prev_time - millis());
  update_prev_time = millis();

  // // Setting controller values.
  // if (theta > 900)  // Dummy value to signal program end.
  // {
  //   pwmL = 0;
  //   pwmR = 0;
  // } else if (theta < 0) {
  //   // Left motor speed up.
  //   pwmL = pwm + Kp * (abs(theta));
  //   pwmR = pwm;
  // } else if (theta > 0) {
  //   // Right motor speed up.
  //   pwmL = pwm + 5;
  //   pwmR = pwm + Kp * (abs(theta));
  // } else {
  //   pwmL = pwm + 5;
  //   pwmR = pwm;
  // }
}