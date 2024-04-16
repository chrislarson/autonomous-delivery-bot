#include <Arduino.h>

#include "led.h"
#include "motors.h"

encoderFrame* leftEncoder = getLeftEncoderFrame();
encoderFrame* rightEncoder = getRightEncoderFrame();

void read_msgs();
void periodic();
void parseData();
void showParsedData();
void recvWithStartEndMarkers();

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
  Serial.println("<c," + String(leftEncoder->pos) + "," + String(rightEncoder->pos) + ">");
}

void sendEncoderCounts(){
  Serial.print("<c,");
  printBytes((void*)&leftEncoder->pos, sizeof leftEncoder->pos);
  Serial.print(",");
  printBytes((void*)&rightEncoder->pos, sizeof leftEncoder->pos);
  Serial.println('>');
}

void showEncoderVelocities(){
  Serial.println("<v," + String(leftEncoder->vel) + "," + String(rightEncoder->vel) + ">");
}

void sendEncoderVelocities(){
  Serial.println("<v," + String(leftEncoder->vel) + "," + String(rightEncoder->vel) + ">");
}

void setup() {
  // Open serial connection.
  Serial.begin(9600);

  // Setup Robot Subsystems
  setupLEDs();
  setupMotors();
}

//============

void loop() {
  read_msgs();
  if (enable) {
    periodic();
  } else {
    LEDRainbow();
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
  updateEncoders();

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