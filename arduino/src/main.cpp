#include <Arduino.h>

// Pin definitions
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
  int pos;
  double vel;
  double acc;
};

// Motor B (LEFT) and encoder B (LEFT)
# define ENL 6
# define dirL1 8
# define dirL2 7
# define encoderLA 2  // A-Phase
# define encoderLB 4  // B-Phase
# define readEncoderLB() (fastDigitalRead(encoderLB))
# define invertLeft true
encoderFrame leftEncoderFrame;

// Motor A (RIGHT) and encoder A (RIGHT)
# define ENR 11
# define dirR1 10
# define dirR2 9
# define encoderRA 3  // A-Phase
# define encoderRB 5  // B-Phase
# define readEncoderRB() (fastDigitalRead(encoderRB))
# define invertRight true
encoderFrame rightEncoderFrame;

// LED Control
# define LED_Enable A0
# define LED_0 A1
# define LED_1 A2
# define LED_2 A3
# define LED_Disable A4
void setLed(byte id);

// LED IDs
# define LED_ERR_ID 1

// global variables to more efficiently control tank drive
bool leftDir = 0;
bool rightDir = 0;

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

// Interrupt callbacks
void indexLeftEncoderCount();
void indexRightEncoderCount();

// Global statics
volatile int_fast32_t leftEncoderCount = 0;
volatile int_fast32_t rightEncoderCount = 0;

void read_msgs();
void periodic();
void parseData();
void showParsedData();
void recvWithStartEndMarkers();
void led_rainbow();

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];  // Temporary array for use while parsing serial com.

boolean parse_error = false;
boolean newData = false;

boolean enable = false;

//============
void printBytes(void* ptr, int len) {
  char bytes[len];
  strncpy(bytes, static_cast<const char*>(static_cast<const void*>(ptr)), len);
  Serial.print(bytes);
}

void showEncoderCounts(){
  Serial.println("<c," + String(leftEncoderCount) + "," + String(rightEncoderCount) + ">");
}

void sendEncoderCounts(){
  Serial.print("<c,");
  printBytes((void*)&leftEncoderCount, sizeof leftEncoderCount);
  Serial.print(",");
  printBytes((void*)&rightEncoderCount, sizeof rightEncoderCount);
  Serial.println('>');
}

void showEncoderVelocities(){
  Serial.println("<v," + String(leftEncoderFrame.vel) + "," + String(rightEncoderFrame.vel) + ">");
}

void sendEncoderVelocities(){
  Serial.println("<v," + String(leftEncoderFrame.vel) + "," + String(rightEncoderFrame.vel) + ">");
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

void setup() {
  // Open serial connection.
  Serial.begin(9600);

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

  // LED Control
  pinMode(LED_Enable, OUTPUT);
  pinMode(LED_0, OUTPUT);
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(LED_Disable, OUTPUT);
  digitalWrite(LED_Disable, LOW);

  // Set initial motor directions
  setLeftDirection(0);
  setRightDirection(0);
}

//============

long previousMillis = 0;
long lastSampleTimeMillis = 0;

void loop() {
  read_msgs();
  if (enable) {
    periodic();
  } else {
    led_rainbow();
  }
  //showParsedData();
}

void read_msgs() {
  // Reading and parsing new data from serial.
  recvWithStartEndMarkers();
  if (newData == true) {
    strcpy(tempChars, receivedChars);
    parseData();
    if (parse_error) {
      setLed(LED_ERR_ID);
      parse_error = false;
    }
    // showParsedData();
    newData = false;
  }
}

void periodic() {
  previousMillis = millis();

  // Update encoder velocity
  int timeSinceEncoderSample = previousMillis - lastSampleTimeMillis;
  if (timeSinceEncoderSample >= encoderSamplingTime) {
    double timeSinceEncoderSampleSec = timeSinceEncoderSample * 1e-3;
    leftEncoderFrame = updateEncoderFrame(&leftEncoderFrame, leftEncoderCount, timeSinceEncoderSampleSec);
    rightEncoderFrame = updateEncoderFrame(&rightEncoderFrame, rightEncoderCount, timeSinceEncoderSampleSec);
    lastSampleTimeMillis = previousMillis;
    //sendEncoderCounts();
    //showEncoderCounts();
    //showEncoderVelocities();
  }

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

byte led_rainbow_state = 0;
int led_rainbow_prev_state_time = 0;
#define LED_RAINBOW_PERIOD 500
void led_rainbow() {
  int currTime = millis();
  if (currTime - led_rainbow_prev_state_time > LED_RAINBOW_PERIOD) {
    setLed(led_rainbow_state+1);
    led_rainbow_state = (led_rainbow_state+1)%8;
    led_rainbow_prev_state_time = currTime;
  }
}

//============

// Reads serial data, expecting format ```<D,{theta:float}>``
void recvWithStartEndMarkers() {
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  while (Serial.available() > 0 && newData == false) {
    char rc = Serial.read();
    if (recvInProgress == true) {
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      } else {
        receivedChars[ndx] = '\0';  // Terminate the string.
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    } else if (rc == startMarker) {
      recvInProgress = true;
    }
  }
}

//============

#define BYTE_LEN 1
byte parseByte() {
  char* data = strtok(NULL, ",");
  if (strnlen(data, BYTE_LEN+1) == BYTE_LEN) {
    return atoi(data);
  } else {
    parse_error = true;
    return 0;
  }
}

#define FLOAT_LEN 4
float parseFloat() {
  char* data = strtok(NULL, ",");
  if (strnlen(data, FLOAT_LEN+1) == FLOAT_LEN) {
    return atof(data);
  } else {
    parse_error = true;
    return 0;
  }
}

// "c,b\0"
#define STATUS_MSG_LEN 3
void parseSetStatus() {
  byte id = parseByte();
  //Serial.println(id);
  if (!parse_error) {
    setLed(id);
  }
}

// "c,f,f\0"
#define PWM_MSG_LEN 12
void parseSetMotorOutput() {
  float leftOutput = parseFloat();
  float rightOutput = parseFloat();
  if (!parse_error) {
    tankDrive(leftOutput, rightOutput);
  }
}

void parseData() {  // Split the serial com data into its parts.
  char cmd;
  // Get first part, the message type.
  cmd = strtok(tempChars, ",")[0];
  //Serial.println(cmd);
  switch (cmd)
  {
  // Enable Message
  case 'e':
  case 'E':
    enable = true;
    Serial.println("<e>");
    setLed(0);
    break;
  // Status Message
  case 's':
  case 'S':
    if (strnlen(receivedChars, numChars) == STATUS_MSG_LEN) {
      parseSetStatus();
    } else {
      //Serial.println(strnlen(receivedChars, numChars));
      parse_error = true;
    }
    break;
  // PWM Message
  case 'p':
  case 'P':
    if (strnlen(tempChars, numChars) == PWM_MSG_LEN) {
      parseSetMotorOutput();
    } else {
      parse_error = true;
    }
    break;
  default:
    parse_error = true;
    break;
  }
}

//============


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
