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

void periodic() {
  //updateEncoders();
  sendSysID();
  Skid_Steer_Update(getLeftEncoderCounts(), getRightEncoderCounts());

  if (Skid_Steer_Get_Control_Mode() == DISABLED){
    float linear;
    float angular;
    bool next = getNextCmd(&linear,&angular);
    if ( fabs(linear) < 1 && next){
        Skid_Steer_Set_Angular_Displacement(0, angular, getLeftEncoderCounts(), getRightEncoderCounts());
    } else if (next){
        Skid_Steer_Set_Displacement(linear, angular, getLeftEncoderCounts(), getRightEncoderCounts());
    }
  }
}