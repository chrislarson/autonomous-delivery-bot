#include "commands.h"

#include "motors.h"
#include "led.h"
#include "skid_steer.h"

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
    Skid_Steer_Set_Enabled(false);
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
    Skid_Steer_Set_Displacement(cmd.data.y_coord, 0, getLeftEncoderCounts(), getRightEncoderCounts());
    Skid_Steer_Set_Enabled(true);
}

void execDisableCmd() {
    DisableCmd cmd;
    cmd.data.cmd = DISABLE;
    cmdReadInto(&cmd, sizeof(cmd));
    sendCommand(DISABLE, &cmd);
    setLed(0);
    tankDrive(0,0);
    Skid_Steer_Set_Enabled(false);
    enabled = false;
}

void execInvalidCmd(int cmd_id) {
    ErrorCmd cmd;
    cmd.data.cmd = ERROR;
    cmd.data.errorCode = INVALID_CMD;
    cmd.data.arg1 = cmd_id;
    cmd.data.arg2 = -1;
    sendCommand(ERROR, &cmd);
    setLed(LED_ERR_ID);
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
    case WAYPOINT:
        if(isEnabled()) execWayPointCmd();
        break;
    case DISABLE:
        if(isEnabled()) execDisableCmd();
        break;
    default:
        execInvalidCmd(nextCmdId());
    }
}

bool isEnabled() {
    return enabled;
}