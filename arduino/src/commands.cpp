#include "commands.h"

#include "motors.h"
#include "led.h"

/// Private Functions

bool enabled = false;
void execEnableCmd() {
    EnableCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    sendCommand(ENABLE, &cmd);
    setLed(0);
    enabled = true;
}

void execStatusCmd() {
    StatusCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    setLed(cmd.status);
}

void execPWMCmd() {
    PwmCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    tankDrive(cmd.left, cmd.right);
    //sendCommand(PWM, &cmd);
}

void execSYS_IDCmd() {
    SysIDCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    // use period to request sys id
    //  every X ms (stored in cmd.period)
    
    // respond with format "Bibbii" -> CMD, time, inputs, outputs
}

void execWayPointCmd() {
    WayPointCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    // convert waypoint into left and right
    //  distance targets and send PWM command
}

/// Public Functions

void execCmd(Command cmd){
    switch (cmd)
    {
    case ENABLE:
        execEnableCmd();
        break;
    case STATUS:
        if(isEnabled()) execStatusCmd();
        break;
    case PWM:
        if(isEnabled()) execPWMCmd();
        break;
    case SYS_ID:
        if(isEnabled()) execSYS_IDCmd();
        break;
    case WAYPOINT:
        if(isEnabled()) execWayPointCmd();
        break;
    }
}

bool isEnabled() {
    return enabled;
}