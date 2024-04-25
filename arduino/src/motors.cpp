#include "motors.h"

#include "serial.h"

volatile int32_t leftEncoderCount = 0;
void indexLeftEncoderCount() {
  byte encoder_phase = (bool) readEncoderLB();
  encoder_phase |= (bool) readEncoderLA() << 1;
  switch (encoder_phase)
  {
  case 0b00:
    leftEncoderCount--;
    break;
  case 0b01:
    leftEncoderCount++;
    break;
  case 0b10:
    leftEncoderCount++;
    break;
  case 0b11:
    leftEncoderCount--;
    break;
  }
  // if (!readEncoderLB()) {
  //   leftEncoderCount++;
  // } else {
  //   leftEncoderCount--;
  // }
}

volatile int32_t rightEncoderCount = 0;
void indexRightEncoderCount() { 
  byte encoder_phase = (bool) readEncoderRB();
  encoder_phase |= (bool) readEncoderRA() << 1;
  switch (encoder_phase)
  {
  case 0b00:
    rightEncoderCount++;
    break;
  case 0b01:
    rightEncoderCount--;
    break;
  case 0b10:
    rightEncoderCount--;
    break;
  case 0b11:
    rightEncoderCount++;
    break;
  }
  // if (readEncoderRB()) {
  //   rightEncoderCount++;
  // } else {
  //   rightEncoderCount--;
  // }
}

void setupMotors() {
    // Motor A (LEFT) pin setup.
    pinMode(ENL, OUTPUT);
    pinMode(dirL1, OUTPUT);
    pinMode(dirL2, OUTPUT);
    pinMode(encoderLA, INPUT_PULLUP);
    pinMode(encoderLB, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderLA), indexLeftEncoderCount, RISING);

    // Motor B (RIGHT) pin setup.
    pinMode(ENR, OUTPUT);
    pinMode(dirR1, OUTPUT);
    pinMode(dirR2, OUTPUT);
    pinMode(encoderRA, INPUT_PULLUP);
    pinMode(encoderRB, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(encoderRA), indexRightEncoderCount, RISING);

    // Set initial motor directions
    setLeftDirection(0);
    setRightDirection(0);
}

bool leftDir = 0;
// 0 = 01, 1 = 10; never set dirL1 and dirL2 to 1 at the same time
void setLeftDirection(bool direction){
  if (direction != invertLeft) {
    digitalWrite(dirL1, 0);
    digitalWrite(dirL2, 1);
  } else {
    digitalWrite(dirL2, 0);
    digitalWrite(dirL1, 1);
  }
  leftDir = direction;
}

bool rightDir = 0;
// 0 = 01, 1 = 10; never set dirL1 and dirL2 to 1 at the same time
void setRightDirection(bool direction){
  if (direction != invertRight) {
    digitalWrite(dirR1, 0);
    digitalWrite(dirR2, 1);
  } else {
    digitalWrite(dirR2, 0);
    digitalWrite(dirR1, 1);
  }
  rightDir = direction;
}

void setMotorOutputs(byte leftVal, byte rightVal) {
  # ifdef LRRatio
  analogWrite(ENR, 255 * rightVal / 100);
  analogWrite(ENL, 255 * leftVal / 100);
  # endif
  # ifdef RLRatio
  analogWrite(ENR, (int) 255 * rightVal * RLRatio);
  analogWrite(ENL, (int) 255 * leftVal);
  # endif
}

// function to control tank drive
int32_t leftPWM = 0;
int32_t rightPWM = 0;
void tankDrive(int16_t leftVal, int16_t rightVal) {
  if ((leftVal < 0) != leftDir) setLeftDirection(leftVal < 0);
  if ((rightVal < 0) != rightDir) setRightDirection(rightVal < 0);
  // leftPWM = (255 * leftVal) / 100;
  // rightPWM = (255 * rightVal) / 100;
  setMotorOutputs(abs(leftPWM), abs(rightPWM));
}

// encoderFrame updateEncoderFrame(const encoderFrame *lastFrame, int newPos, double timeSinceLast) {
//   encoderFrame newFrame;
//   newFrame.pos = newPos;
//   newFrame.vel = (double)(newFrame.pos - lastFrame->pos) / timeSinceLast;
//   newFrame.acc = (newFrame.vel - lastFrame->vel) / timeSinceLast;
//   return newFrame;
// }

// encoderFrame leftEncoderFrame;
// encoderFrame rightEncoderFrame;
// int lastSampleTimeMillis = millis();
// void updateEncoders() {
//     int currMillis = millis();

//     // Update encoder velocity
//     int timeSinceEncoderSample = currMillis - lastSampleTimeMillis;
//     if (timeSinceEncoderSample >= encoderSamplingTime) {
//         double timeSinceEncoderSampleSec = timeSinceEncoderSample * 1e-3;
//         updateEncoderFrame(&leftEncoderFrame, leftEncoderCount, timeSinceEncoderSampleSec);
//         updateEncoderFrame(&rightEncoderFrame, rightEncoderCount, timeSinceEncoderSampleSec);
//         lastSampleTimeMillis = currMillis;
//     }
// }

// encoderFrame* getLeftEncoderFrame() {
//     return &leftEncoderFrame;
// }

// encoderFrame* getRightEncoderFrame() {
//     return &rightEncoderFrame;
// }

float getLeftEncoderCounts() {
  return leftEncoderCount;
}

float getRightEncoderCounts() {
  return rightEncoderCount;
}

int16_t sys_id_period = -1;
void setSysIDPeriod(int16_t period) {
  sys_id_period = period;
}

unsigned long sys_id_prev_time = 0;
void sendSysID() {
  if (sys_id_period >= 0) {
    unsigned long currTime = millis();
    unsigned long deltaTime = currTime - sys_id_prev_time;
    if (deltaTime >= (unsigned long) sys_id_period) {
      sys_id_prev_time = currTime;
      SysResponseCmd sysResponseCmd;
      sysResponseCmd.data.cmd = SYS_RESPONSE;
      sysResponseCmd.data.time_millis = currTime;
      sysResponseCmd.data.left_pwm = leftPWM;
      sysResponseCmd.data.right_pwm = rightPWM;
      sysResponseCmd.data.left_enc = getLeftEncoderCounts();
      sysResponseCmd.data.right_enc = getRightEncoderCounts();
      sendCommand(SYS_RESPONSE, &sysResponseCmd);
    }
  }
}