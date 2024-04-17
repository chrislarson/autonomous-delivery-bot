#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>

enum Command {
    ENABLE = 1,
    STATUS = 2,
    PWM = 3,
};

struct __attribute__( ( __packed__ ) ) EnableCmd {
    unsigned char cmd;
};

struct __attribute__( ( __packed__ ) ) StatusCmd {
    unsigned char cmd;
    unsigned char status;
};

struct __attribute__( ( __packed__ ) ) PwmCmd {
    unsigned char cmd;
    signed char left;
    signed char right;
};

// Functions
void setupSerial();
bool updateSerial();
void sendCommand(Command cmd, void* cmdStruct);
Command nextCmdType();
bool cmdReadInto(void* v_ptr, byte len);

#endif