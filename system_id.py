# Send CMD, PWM_L, PWM_R,
from typing import Any
import serial
from scipy.signal import chirp, spectrogram
import matplotlib.pyplot as plt
import time
import os
from step_trajectory import StepTrajectory
from chirp_trajectory import ChirpTrajectory

from arduino.serialio import sendCommand, receiveCommand, Command, enableArduino


_SERIAL_ENABLED = False
_CHIRP_TRAJECTORY = True


if __name__ == "__main__":

    # 0. Create a directory to store data.
    test_id = "sysid_" + str(int(time.monotonic()))
    test_dir = os.path.join("data", test_id)
    os.mkdir(test_dir)

    # 0A. Open an output file to receive
    outfile_path = os.path.join(test_dir, "output.csv")
    outfile = open(outfile_path, "w")
    header_string = "cmd,time,pwmL,pwmR,encL,encR\n"
    outfile.write(header_string)

    # Configure system ID
    TEST_DURATION_SEC = 20
    DT_SEC = 0.02

    # 1A. Generate step trajectory.
    if _CHIRP_TRAJECTORY:
        chirp_traj = ChirpTrajectory(test_dir, TEST_DURATION_SEC, DT_SEC, 100, 10)
        traj = chirp_traj.generate_trajectory(
            test_dir, TEST_DURATION_SEC, dt_sec=DT_SEC
        )
        chirp_traj.plot_trajectory(test_dir, True)
    else:
        step_traj = StepTrajectory(
            test_dir, TEST_DURATION_SEC, DT_SEC, 100, 10, True, True
        )
        traj = step_traj.generate_trajectory(test_dir)
        step_traj.plot_trajectory(test_dir, True)

    # 1B. Generate chirp trajectory

    # 2. Connect to serial.
    if _SERIAL_ENABLED:
        ser = serial.Serial("/dev/ttyACM0", 115200, timeout=1)
        ser.reset_input_buffer()
        ser.read_all()
        enableArduino(ser)

        msg = sendCommand(ser, Command.SYS_ID, 10)
        # print(msg)

    # 3. Iterate through trajectory
    # -> Reads in received lines on every iteration.
    # -> Sends new PWM commands when command has changed (new PMW != prev PWM)
    sys_id_in_progress = True

    cmd_idx = 0
    next_cmd = traj[cmd_idx, :]
    start_time = time.monotonic_ns()
    last_sent_cmd = None

    try:
        while sys_id_in_progress:

            # 1. Read in serial & write to output file.
            if _SERIAL_ENABLED:
                msg = receiveCommand(ser)
                if not msg is None and msg[0] == Command.SYS_RESPONSE.value:
                    row_string = "{},{},{},{},{},{}\n".format(
                        msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]
                    )
                    outfile.write(row_string)
                    # print(msg[1:])

            cmd_time = next_cmd[0]
            dt_ns = time.monotonic_ns() - start_time
            if dt_ns >= cmd_time * 1_000_000_000:
                if (
                    last_sent_cmd is None
                    or last_sent_cmd[1] != next_cmd[1]
                    or last_sent_cmd[2] != next_cmd[2]
                ):
                    # 2. Send via serial.
                    if _SERIAL_ENABLED:
                        print(
                            sendCommand(
                                ser, Command.PWM, int(next_cmd[1]), int(next_cmd[2])
                            )
                        )
                        last_sent_cmd = next_cmd

                cmd_idx += 1
                if cmd_idx >= len(traj):
                    sys_id_in_progress = False
                else:
                    next_cmd = traj[cmd_idx, :]
    except KeyboardInterrupt:
        msg = sendCommand(ser, Command.DISABLE)
        sys_id_in_progress = False
        outfile.close()
        exit()

    # 3. Final read from serial and close out program.
    # if _SERIAL_ENABLED:
    #     msg = receiveCommand(ser)
    #     while not msg is None:
    #         if msg[0] == Command.SYS_RESPONSE:
    #             print(msg[1:])
    #         msg = receiveCommand(ser)

    msg = sendCommand(ser, Command.DISABLE)
    outfile.close()
