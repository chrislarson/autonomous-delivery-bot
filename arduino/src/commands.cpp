#include "commands.h"

#include "motors.h"
#include "led.h"

/// Private Functions

bool enabled = false;
void execEnableCmd() {
    EnableCmd cmd;
    cmd.data.cmd = ENABLE;
    cmdReadInto(&cmd, sizeof(cmd));
    sendCommand(ENABLE, &cmd);
    setLed(0);
    enabled = true;
}

void execStatusCmd() {
    StatusCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    setLed(cmd.data.status);
}

void execPWMCmd() {
    PwmCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    tankDrive(cmd.data.left, cmd.data.right);
    //sendCommand(PWM, &cmd);
}

void execSysIDCmd() {
    SysIDCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    setSysIDPeriod(cmd.data.period);
    // sendCommand(SYS_ID, &cmd);
    sendSysID();
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
void execDisableCmd() {
    DisableCmd cmd;
    cmd.data.cmd = DISABLE;
    cmdReadInto(&cmd, sizeof(cmd));
    sendCommand(DISABLE, &cmd);
    setLed(0);
    tankDrive(0,0);
    enabled = false;
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
        if(isEnabled()) execSysIDCmd();
        break;
    case SYS_RESPONSE:
        break;
    case WAYPOINT:
        if(isEnabled()) execWayPointCmd();
        break;
    case DISABLE:
        if(isEnabled()) execDisableCmd();
        break;
    case ERROR:
        break;
    }
}

bool isEnabled() {
    return enabled;
}