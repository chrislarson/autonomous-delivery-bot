import serial
import struct
import time
from enum import Enum


class Command(Enum):
    ENABLE = 1
    STATUS = 2
    PWM = 3
    SYS_ID = 4
    SYS_RESPONSE = 5
    WAYPOINT = 6
    DISABLE = 7
    ERROR = 8
    DISP = 9
    # skip 10, because it gets read as '\n'
    TRAJ_START = 11
    DEBUG = 12


class Error(Enum):
    PARSE = 1
    BUFFER_OVERFLOW = 2
    SIZE_MISMATCH = 3
    EXEC_FAIL = 4
    INVALID_CMD = 5


cmd_fmts = {
    Command.ENABLE: "B",
    Command.STATUS: "BB",
    Command.PWM: "Bbb",
    Command.SYS_ID: "Bi",
    Command.WAYPOINT: "Bff",
    Command.SYS_RESPONSE: "BIiiii",
    Command.DISABLE: "B",
    Command.ERROR: "BBii",
    Command.DISP: "Bff",
    Command.TRAJ_START: "BB",
    Command.DEBUG: "BIiiii",
}


def decodeError(err: Error, *args):
    match err:
        case Error.PARSE:
            print("ERROR: PARSE ERROR")
        case Error.BUFFER_OVERFLOW:
            print("ERROR: BUFFER OVERFLOW")
        case Error.SIZE_MISMATCH:
            print(
                "ERROR: SIZE MISMATCH, expected size: %i, received size: %i"
                % (args[0], args[1])
            )
        case Error.EXEC_FAIL:
            print("ERROR: EXECUTION FAILED, ID: %i" % (args[0]))
        case Error.INVALID_CMD:
            print("ERROR: INVALID COMMAND, ID: %i" % (args[0]))
        case default:
            print("ERROR: Unknown")


def genCmd(cmd: Command, *args):
    return struct.pack("<" + cmd_fmts[cmd], cmd.value, *args) + b"\n"


def sendCommand(ser: serial.Serial, cmd: Command, *args):
    msg = genCmd(cmd, *args)
    ser.write(msg)
    return msg


def receiveCommand(ser: serial.Serial):
    cmd = ser.read(size=1)
    if len(cmd) != 1 or cmd == 0:
        print("CMD ERR:", len(cmd), ",", cmd)
        return None
    msg_len = struct.calcsize("<" + cmd_fmts[Command(cmd[0])])
    msg = ser.read(msg_len)
    if len(msg) != msg_len or msg[-1] != ord("\n"):
        print("MSG ERR:", len(msg), ",", msg)
        print(msg_len, "!=", len(msg))
        print(msg[-1], "!=", ord("\n"))
        return None

    # print(len(msg), msg)
    # msg = msg.rstrip()
    try:
        unpacked = struct.unpack("<" + cmd_fmts[Command(cmd[0])], cmd + msg.rstrip())
        if unpacked[0] == Command.ERROR:
            decodeError(unpacked[1], unpacked[2:])
        else:
            return unpacked
    except struct.error as e:
        print(e)
        print(len(msg), ",", msg)
        return None
    except ValueError as e:
        print(e)
        print(len(msg), ",", msg)
        return None


def enableArduino(ser: serial.Serial):
    response = None
    while response is None or response[0] != 1:
        msg = sendCommand(ser, Command.ENABLE)
        print(msg)

        # Read response
        response = receiveCommand(ser)
        print(response)


if __name__ == "__main__":

    # ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
    ser = serial.Serial("/dev/tty.usbmodem101", 115200, timeout=1)
    ser.reset_input_buffer()

    enableArduino(ser)

    # Send new instruction
    # msg = sendCommand(ser, Command.ERROR, 2, 2, 2)
    # print(msg)

    # response = receiveCommand(ser)
    # print(response)

    # msg = sendCommand(ser, Command.SYS_ID, 20)
    # print(msg)

    # msg = sendCommand(ser, Command.WAYPOINT, 0.0, 1000)
    # print(msg)

    msg = sendCommand(ser, Command.TRAJ_START, 2)
    print(msg)

    msg = sendCommand(ser, Command.DISP, 2000, 0)
    print(msg)

    msg = sendCommand(ser, Command.DISP, 0, 0)
    print(msg)

    # msg = sendCommand(ser, Command.DISABLE)
    # print(msg)

    try:
        while True:
            # Read response
            response = receiveCommand(ser)
            print(response)
    except KeyboardInterrupt:
        msg = sendCommand(ser, Command.DISABLE)
        print(msg)

    #     # # Send new instruction
    #     # msg = sendCommand(ser, Command.STATUS, i+1)
    #     # print(msg)

    # time.sleep(0.1)
    #     # i = (i+1) % 8
