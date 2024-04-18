#include "serial.h"

#define receive_buffer_size sizeof(PwmCmd)+1
byte receive_buffer[receive_buffer_size];
byte receive_buffer_len;

/// Private Functions

void echoCommand() {
    Command cmd = static_cast<Command>(receive_buffer[0]);
    switch (cmd)
    {
    case ENABLE:
    {
        EnableCmd enableCmd;
        cmdReadInto(&enableCmd, sizeof(enableCmd));
        sendCommand(cmd, &enableCmd);
        break;
    }
    case STATUS:
    {
        StatusCmd statusCmd;
        cmdReadInto(&statusCmd, sizeof(statusCmd));
        sendCommand(cmd, &statusCmd);
        break;
    }
    case PWM:
    {
        PwmCmd pwmCmd;
        cmdReadInto(&pwmCmd, sizeof(pwmCmd));
        sendCommand(cmd, &pwmCmd);
        break;
    }
    case SYS_ID:
    {
        SysIDCmd sysIdCmd;
        cmdReadInto(&sysIdCmd, sizeof(sysIdCmd));
        sendCommand(cmd, &sysIdCmd);
        break;
    }
    case WAYPOINT:
    {
        WayPointCmd wayPointCmd;
        cmdReadInto(&wayPointCmd, sizeof(wayPointCmd));
        sendCommand(cmd, &wayPointCmd);
        break;
    }
    }
}

/// Public Functions

void setupSerial() {
    // Open serial connection.
    Serial.begin(9600);
    receive_buffer_len = 0;
}

bool updateSerial() {
    while(Serial.available()) {
        receive_buffer[receive_buffer_len] = Serial.read();
        receive_buffer_len++;
        if (receive_buffer[receive_buffer_len-1] == '\n') {
            //echoCommand();
            return true;
        } else if (receive_buffer_len >= receive_buffer_size) {
            receive_buffer_len = 0;
            return false;
        }
    }
    return false;
}

#define maxFmtLen 4
void sendCommand(Command cmd, void* cmdStruct) {
    switch (cmd)
    {
    case ENABLE:
    {
        EnableCmd* enableCmd = (EnableCmd*) cmdStruct;
        Serial.write(enableCmd->cmd);
        break;
    }
    case STATUS:
    {
        StatusCmd* statusCmd = (StatusCmd*) cmdStruct;
        Serial.write(statusCmd->cmd);
        Serial.write(statusCmd->status);
        break;
    }
    case PWM:
    {
        PwmCmd* pwmCmd = (PwmCmd*) cmdStruct;
        Serial.write(pwmCmd->cmd);
        Serial.write(pwmCmd->left);
        Serial.write(pwmCmd->right);
        break;
    }
    case SYS_ID:
    {
        SysIDCmd* sysIDCmd = (SysIDCmd*) cmdStruct;
        Serial.write(sysIDCmd->cmd);
        Serial.write(sysIDCmd->period);
        break;
    }
    case WAYPOINT:
    {
        WayPointCmd* wayPointCmd = (WayPointCmd*) cmdStruct;
        Serial.write(wayPointCmd->cmd);
        Serial.write(&wayPointCmd->x_coord.bt[0], 4);
        Serial.write(&wayPointCmd->y_coord.bt[0], 4);
        break;
    }
    }

    Serial.write('\n');
    Serial.flush();
}

Command nextCmdType() {
    return static_cast<Command>(receive_buffer[0]);
}

bool cmdReadInto(void* v_ptr, byte len) {
    if (receive_buffer_len-1 == len) {
        byte* b_ptr = (byte*) v_ptr;
        for( uint8_t i = 0; i < len; i++ ) {
            b_ptr[i] = receive_buffer[i];
        }
        receive_buffer_len = 0;
        return true;
    } else {
        return false;
    }
}