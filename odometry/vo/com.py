import random

import serial


def get_random_message() -> bytes:
    instructions = [
        b"<I_SetPWM,125,125>",
        b"<I_SetPWM,150,150>",
        b"<I_SetPWM,125,125>",
        b"<I_SetPWM,100,100>",
        b"<I_ResetEncoder,0,0>",
    ]
    return random.choice(instructions)


if __name__ == "__main__":
    ser = serial.Serial("/dev/cu.usbmodem1201", 9600, timeout=1)
    ser.reset_input_buffer()

    while True:
        # Send new instruction
        instruction = get_random_message()
        ser.write(instruction)

        # Read encoder state
        # this can crash...
        encoder_state = ser.readline()
        # print(len(encoder_state))

        encoder_state_str = encoder_state.decode("utf-8").rstrip()

        print(encoder_state)
        # time.sleep(0.5)
