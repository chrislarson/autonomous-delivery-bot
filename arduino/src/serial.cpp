#include "serial.h"

#define receive_buffer_size sizeof(WayPointCmd)+1
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
    case SYS_RESPONSE:
        SysResponseCmd sysResponseCmd;
        cmdReadInto(&sysResponseCmd, sizeof(sysResponseCmd));
        sendCommand(cmd, &sysResponseCmd);
    case WAYPOINT:
    {
        WayPointCmd wayPointCmd;
        cmdReadInto(&wayPointCmd, sizeof(wayPointCmd));
        sendCommand(cmd, &wayPointCmd);
        break;
    }
    case DISABLE:
    {
        DisableCmd disableCmd;
        cmdReadInto(&disableCmd, sizeof(disableCmd));
        sendCommand(cmd, &disableCmd);
        break;
    }
    case ERROR:
        ErrorCmd errorCmd;
        cmdReadInto(&errorCmd, sizeof(errorCmd));
        sendCommand(cmd, &errorCmd);
        break;
    }
}

int cmdSize(Command cmd) {
    switch (cmd)
    {
    case ENABLE:
        return sizeof(EnableCmd);
    case STATUS:
        return sizeof(StatusCmd);
    case PWM:
        return sizeof(PwmCmd);
    case SYS_ID:
        return sizeof(SysIDCmd);
    case SYS_RESPONSE:
        return sizeof(SysResponseCmd);
    case WAYPOINT:
        return sizeof(WayPointCmd);
    case DISABLE:
        return sizeof(DisableCmd);
    case ERROR:
        return sizeof(ErrorCmd);
    default:
        return -1;
    }
}

/// Public Functions

void setupSerial() {
    // Open serial connection.
    Serial.begin(115200);
    receive_buffer_len = 0;
}

bool updateSerial() {
    while(Serial.available()) {
        receive_buffer[receive_buffer_len] = Serial.read();
        receive_buffer_len++;
        if (receive_buffer[receive_buffer_len-1] == '\n') {
            //echoCommand();
            int cmd_len = cmdSize(nextCmdType())+1;
            return cmd_len == receive_buffer_len;
        } else if (receive_buffer_len >= receive_buffer_size) {
            receive_buffer_len = 0;
            return false;
        }
    }
    return false;
}

void sendCommand(Command cmd, void* cmdStruct) {
    switch (cmd)
    {
    case ENABLE:
    {
        EnableCmd* enableCmd = (EnableCmd*) cmdStruct;
        Serial.write(&enableCmd->raw[0], sizeof(enableCmd->raw));
        break;
    }
    case STATUS:
    {
        StatusCmd* statusCmd = (StatusCmd*) cmdStruct;
        Serial.write(&statusCmd->raw[0], sizeof(statusCmd->raw));
        break;
    }
    case PWM:
    {
        PwmCmd* pwmCmd = (PwmCmd*) cmdStruct;
        Serial.write(&pwmCmd->raw[0], sizeof(pwmCmd->raw));
        break;
    }
    case SYS_ID:
    {
        SysIDCmd* sysIDCmd = (SysIDCmd*) cmdStruct;
        Serial.write(&sysIDCmd->raw[0], sizeof(sysIDCmd->raw));
        break;
    }
    case SYS_RESPONSE:
    {
        SysResponseCmd* sysResponseCmd = (SysResponseCmd*) cmdStruct;
        Serial.write(&sysResponseCmd->raw[0], sizeof(sysResponseCmd->raw));
        break;
    }
    case WAYPOINT:
    {
        WayPointCmd* wayPointCmd = (WayPointCmd*) cmdStruct;
        Serial.write(&wayPointCmd->raw[0], sizeof(wayPointCmd->raw));
        break;
    }
    case DISABLE:
    {
        DisableCmd* disableCmd = (DisableCmd*) cmdStruct;
        Serial.write(&disableCmd->raw[0], sizeof(disableCmd->raw));
        break;
    }
    case ERROR:
        ErrorCmd* errorCmd = (ErrorCmd*) cmdStruct;
        Serial.write(&errorCmd->raw[0], sizeof(errorCmd->raw));
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