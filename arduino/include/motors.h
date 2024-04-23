#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>

// MOTOR WIRES

// Black - Ground
// Red - Vcc

// White - Sensor Ground
// Yellow - Sensor Vcc
// Orange - A-Phase
// Green - B-Phase

// fast version of digital read that reads the input register directly. For use with macros.
# define fastDigitalRead(P) (*portInputRegister(digitalPinToPort(P)) & digitalPinToBitMask(P))

struct encoderFrame
{
int pos = 0;
double vel = 0.0;
double acc = 0.0;
};

// Motor B (LEFT) and encoder B (LEFT)
# define ENL 6
# define dirL1 8
# define dirL2 7
# define encoderLA 3  // A-Phase
# define encoderLB 5  // B-Phase
# define readEncoderLA() (fastDigitalRead(encoderLA))
# define readEncoderLB() (fastDigitalRead(encoderLB))
# define invertLeft false


// Motor A (RIGHT) and encoder A (RIGHT)
# define ENR 11
# define dirR1 10
# define dirR2 9
# define encoderRA 2  // A-Phase
# define encoderRB 4  // B-Phase
# define readEncoderRA() (fastDigitalRead(encoderRA))
# define readEncoderRB() (fastDigitalRead(encoderRB))
# define invertRight false

// Drive train characteristics
#define LRRatio 1.0 // value <= 1.0; multiply rightVal by LRRatio to get equivalent leftVal
// #define RLRatio 1.0 // value <= 1.0; multiply leftVal by LRRatio to get equivalent rightVal
#define encoderSamplingTime 100 // sampling time in milliseconds
// Wheel characteristics
#define countsPerRev 240
#define distPerRev 2394  // mm

// PD controller gains.
#define Kp 2
#define Kd 1

// Functions
void setupMotors();
void setLeftDirection(bool direction);
void setRightDirection(bool direction);
void tankDrive(int8_t leftVal, int8_t rightVal);
void updateEncoders();
encoderFrame* getLeftEncoderFrame();
encoderFrame* getRightEncoderFrame();
void setSysIDPeriod(int period);
void sendSysID();

#endif