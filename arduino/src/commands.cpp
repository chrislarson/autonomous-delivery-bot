#include "commands.h"

#include "motors.h"
#include "led.h"
#include "skid_steer.h"

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
    setLed(cmd.data.status);
}

void execPWMCmd() {
    PwmCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    Skid_Steer_Disable();
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
    //Skid_Steer_Set_Velocity(cmd.data.y_coord, 0);
    Skid_Steer_Set_Displacement(cmd.data.y_coord, 0, getLeftEncoderCounts(), getRightEncoderCounts());
}

void execDisableCmd() {
    DisableCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    sendCommand(DISABLE, &cmd);
    setLed(0);
    tankDrive(0,0);
    Skid_Steer_Disable();
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

void execDispCmd(){
    DispCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
    sendCommand(DISP, &cmd);
    if (cmd.data.lin_disp < 1){
        Skid_Steer_Set_Angular_Displacement(0, cmd.data.ang_disp, getLeftEncoderCounts(), getRightEncoderCounts());
    } else{
        Skid_Steer_Set_Displacement(cmd.data.lin_disp, cmd.data.ang_disp, getLeftEncoderCounts(), getRightEncoderCounts());
    }
    
}

void execTrajStartCmd() {
    TrajStartCmd cmd;
    cmdReadInto(&cmd, sizeof(cmd));
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
    case DISP:
        if(isEnabled()) execDispCmd();
        break;
    case TRAJ_START:
        if(isEnabled()) execTrajStartCmd();
        break;
    default:
        execInvalidCmd(nextCmdId());
    }
}

bool isEnabled() {
    return enabled;
}