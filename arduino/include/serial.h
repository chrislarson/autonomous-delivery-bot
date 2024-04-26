#ifndef SERIAL_H
#define SERIAL_H

#include <Arduino.h>

enum Command {
    ENABLE = 1,
    STATUS = 2,
    PWM = 3,
    SYS_ID = 4,
    SYS_RESPONSE = 5,
    WAYPOINT = 6,
    DISABLE = 7,
    ERROR = 8,
    DISP = 9
};

enum Error {
    PARSE = 1,
    BUFFER_OVERFLOW = 2,
    SIZE_MISMATCH = 3,
    EXEC_FAIL = 4,
    INVALID_CMD = 5
};

union EnableCmd {
    struct __attribute__( ( __packed__ ) ) {
        unsigned char cmd;
    } data;
    byte raw[1];
};

union StatusCmd {
    struct __attribute__( ( __packed__ ) ) {
        unsigned char cmd;
        unsigned char status;
    } data;
    byte raw[2];
};

union PwmCmd {
    struct __attribute__( ( __packed__ ) ) {
        unsigned char cmd;
        signed char left;
        signed char right;
    } data;
    byte raw[3];
};

union SysIDCmd {
    struct __attribute__( ( __packed__ ) ) {
        unsigned char cmd;
        int32_t period;
    } data;
    byte raw[5];  
};

union SysResponseCmd {
    struct __attribute__( ( __packed__ ) ) {
        unsigned char cmd;
        uint32_t time_millis;
        int32_t left_pwm;
        int32_t right_pwm;
        int32_t left_enc;
        int32_t right_enc;
    } data;
    byte raw[21];
};

union WayPointCmd
{
    struct __attribute__( ( __packed__ ) ) {
        unsigned char cmd;
        float x_coord;
        float y_coord;
    } data;
    byte raw[9];
};

union DisableCmd {
    struct __attribute__( ( __packed__ ) ) {
        unsigned char cmd;
    } data;
    byte raw[1];
};

union ErrorCmd {
    struct __attribute__( ( __packed__ ) ) {
        unsigned char cmd;
        unsigned char errorCode;
        int32_t arg1;
        int32_t arg2;
    } data;
    byte raw[10];
};

union DispCmd{
    struct __attribute__ ( ( __packed__ ) ) {
        unsigned char cmd;
        float lin_disp;
        float ang_disp;
    } data;
    byte raw[9];
};

// Functions
void setupSerial();
bool updateSerial();
void sendCommand(Command cmd, void* cmdStruct);
int nextCmdId();
Command nextCmdType();
bool cmdReadInto(void* v_ptr, byte len);

#endif