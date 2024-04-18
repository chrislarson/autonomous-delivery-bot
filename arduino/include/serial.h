#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>

enum Command {
    ENABLE = 1,
    STATUS = 2,
    PWM = 3,
    SYS_ID = 4,
    SYS_RESPONSE = 5,
    WAYPOINT = 6
};

typedef union
{
    float fp;
    byte bt[4];
} binaryFloat;


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

struct __attribute__( ( __packed__ ) ) SysIDCmd {
    unsigned char cmd;
    signed short period;
};

struct __attribute__( ( __packed__ ) ) WayPointCmd {
    unsigned char cmd;
    binaryFloat x_coord;
    binaryFloat y_coord;
};

// Functions
void setupSerial();
bool updateSerial();
void sendCommand(Command cmd, void* cmdStruct);
Command nextCmdType();
bool cmdReadInto(void* v_ptr, byte len);

#endif