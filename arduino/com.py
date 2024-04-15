import random

import serial
import time


def get_random_message() -> bytes:
    instructions = [
        b"<s,2>",
        b"<s,3>",
        b"<s,4>",
        b"<s,5>",
        b"<s,6>",
        b"<s,7>",
        b"<s,8>",
    ]
    return random.choice(instructions)


if __name__ == "__main__":
    ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
    ser.reset_input_buffer()

    while True:
        # Send new instruction
        instruction = get_random_message()
        ser.write(instruction)
        print(instruction)

        # Read encoder state
        # this can crash...
        encoder_state = ser.readline()
        encoder_state += ser.readline()
        # print(len(encoder_state))

        encoder_state_str = encoder_state.decode("utf-8").rstrip()

        print(encoder_state)
        time.sleep(3)
