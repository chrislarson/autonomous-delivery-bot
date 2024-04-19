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


cmd_fmts = {
    Command.ENABLE: "B",
    Command.STATUS: "BB",
    Command.PWM: "Bbb",
    Command.SYS_ID: "Bi",
    Command.WAYPOINT: "Bff",
    Command.SYS_RESPONSE: "BIiiii",
}


def genCmd(cmd: Command, *args):
    return struct.pack("<" + cmd_fmts[cmd], cmd.value, *args) + b'\n'


def sendCommand(ser: serial.Serial, cmd: Command, *args):
    msg = genCmd(cmd, *args)
    ser.write(msg)
    return msg


def recieveCommand(ser: serial.Serial):
    msg = ser.readline()
    if len(msg) > 0:
        print(len(msg), msg)
        msg = msg.rstrip()
        return struct.unpack("<" + cmd_fmts[Command(msg[0])], msg)
    else:
        return None


if __name__ == "__main__":
    ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
    ser.reset_input_buffer()

    response = None
    while (response is None):
        msg = sendCommand(ser, Command.ENABLE)
        print(msg)

        # Read response
        response = recieveCommand(ser)
        print(response)

    # Send new instruction
    msg = sendCommand(ser, Command.SYS_ID, 1000)
    print(msg)

    msg = sendCommand(ser, Command.PWM, 0, 0)
    print(msg)

    # i = 0
    while True:
        # Read response
        response = recieveCommand(ser)
        print(response)

        # # Send new instruction
        # msg = sendCommand(ser, Command.STATUS, i+1)
        # print(msg)

        time.sleep(0.1)
        # i = (i+1) % 8
