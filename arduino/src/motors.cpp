#include "motors.h"

bool leftDir = 0;
encoderFrame leftEncoderFrame;
volatile int leftEncoderCount = 0;

bool rightDir = 0;
encoderFrame rightEncoderFrame;
volatile int rightEncoderCount = 0;

void indexLeftEncoderCount() { 
  if (!readEncoderLB()) {
    leftEncoderCount++;
  } else {
    leftEncoderCount--;
  }
}

void indexRightEncoderCount() { 
  if (readEncoderRB()) {
    rightEncoderCount++;
  } else {
    rightEncoderCount--;
  }
}

void setupMotors() {
    // Motor A (LEFT) pin setup.
    pinMode(ENL, OUTPUT);
    pinMode(dirL1, OUTPUT);
    pinMode(dirL2, OUTPUT);
    pinMode(encoderLA, INPUT);
    pinMode(encoderLB, INPUT);
    attachInterrupt(digitalPinToInterrupt(encoderLA), indexLeftEncoderCount, RISING);

    // Motor B (RIGHT) pin setup.
    pinMode(ENR, OUTPUT);
    pinMode(dirR1, OUTPUT);
    pinMode(dirR2, OUTPUT);
    pinMode(encoderRA, INPUT);
    pinMode(encoderRB, INPUT);
    attachInterrupt(digitalPinToInterrupt(encoderRA), indexRightEncoderCount, RISING);

    // Set initial motor directions
    setLeftDirection(0);
    setRightDirection(0);
}

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

void setMotorOutputs(float leftVal, float rightVal) {
  # ifdef LRRatio
  analogWrite(ENR, (int) 255 * rightVal);
  analogWrite(ENL, (int) 255 * leftVal * LRRatio);
  # endif
  # ifdef RLRatio
  analogWrite(ENR, (int) 255 * rightVal * RLRatio);
  analogWrite(ENL, (int) 255 * leftVal);
  # endif
}

// function to control tank drive
void tankDrive(float leftVal, float rightVal) {
  if ((leftVal < 0) != leftDir) setLeftDirection(leftVal < 0);
  if ((rightVal < 0) != rightDir) setRightDirection(rightVal < 0);
  setMotorOutputs(abs(leftVal), abs(rightVal));
}

encoderFrame updateEncoderFrame(const encoderFrame *lastFrame, int newPos, double timeSinceLast) {
  encoderFrame newFrame;
  newFrame.pos = newPos;
  newFrame.vel = (double)(newFrame.pos - lastFrame->pos) / timeSinceLast;
  newFrame.acc = (newFrame.vel - lastFrame->vel) / timeSinceLast;
  return newFrame;
}

int lastSampleTimeMillis = millis();
void updateEncoders() {
    int currMillis = millis();

    // Update encoder velocity
    int timeSinceEncoderSample = currMillis - lastSampleTimeMillis;
    if (timeSinceEncoderSample >= encoderSamplingTime) {
        double timeSinceEncoderSampleSec = timeSinceEncoderSample * 1e-3;
        updateEncoderFrame(&leftEncoderFrame, leftEncoderCount, timeSinceEncoderSampleSec);
        updateEncoderFrame(&rightEncoderFrame, rightEncoderCount, timeSinceEncoderSampleSec);
        lastSampleTimeMillis = currMillis;
    }
}

encoderFrame* getLeftEncoderFrame() {
    return &leftEncoderFrame;
}

encoderFrame* getRightEncoderFrame() {
    return &rightEncoderFrame;
}