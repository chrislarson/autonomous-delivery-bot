#include <Arduino.h>

#include "serial.h"
#include "commands.h"
#include "led.h"
#include "motors.h"
#include "controller.h"

encoderFrame* leftEncoder = getLeftEncoderFrame();
encoderFrame* rightEncoder = getRightEncoderFrame();

void periodic();

//============

void setup() {
  // Setup Robot Subsystems
  setupSerial();
  setupLEDs();
  setupMotors();
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

void periodic() {
  updateEncoders();
  sendSysID();

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