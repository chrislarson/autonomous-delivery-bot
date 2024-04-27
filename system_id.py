from typing import Any, Union
import serial
from scipy.signal import chirp, spectrogram
import matplotlib.pyplot as plt
import time
import os
from step_trajectory import StepTrajectory
from chirp_trajectory import ChirpTrajectory
import numpy as np

from arduino.serialio import sendCommand, receiveCommand, Command, enableArduino


class SystemID:

    def id_sys(self, traj_path: str, ser_conn: Union[serial.Serial, None]):
        outfile = None

        try:
            traj = np.loadtxt(traj_path, delimiter=",")
            traj_parent_dir = os.path.dirname(traj_path)

            #
            if ser_conn is not None:
                ser_conn.reset_input_buffer()
                ser_conn.read_all()
                enableArduino(ser_conn)
                msg = sendCommand(ser_conn, Command.SYS_ID, 10)

            # Open an output file to receive sysid data.
            outfile_path = os.path.join(traj_parent_dir, "output.csv")
            outfile = open(outfile_path, "w")
            header_string = "cmd,time,pwmL,pwmR,encL,encR\n"
            outfile.write(header_string)

            # Loop through trajectory and send (or print) commands at intended time.
            start_ts = time.monotonic_ns()
            cmd_idx = 0
            last_sent_cmd = None
            while cmd_idx < len(traj):

                if ser_conn is not None:
                    msg = receiveCommand(ser_conn)
                    if not msg is None and msg[0] == Command.SYS_RESPONSE.value:
                        row_string = "{},{},{},{},{},{}\n".format(
                            msg[0], msg[1], msg[2], msg[3], msg[4], msg[5]
                        )
                        outfile.write(row_string)

                next_cmd = traj[cmd_idx, :]
                cmd_time = next_cmd[0]
                dt_ns = time.monotonic_ns() - start_ts
                if dt_ns >= cmd_time * 1_000_000_000:
                    # Only send the command if it has changed from previous.
                    if (
                        last_sent_cmd is None
                        or last_sent_cmd[1] != next_cmd[1]
                        or last_sent_cmd[2] != next_cmd[2]
                    ):
                        if ser_conn is not None:
                            print(
                                sendCommand(
                                    ser_conn,
                                    Command.PWM,
                                    int(next_cmd[1]),
                                    int(next_cmd[2]),
                                )
                            )
                        else:
                            print(next_cmd)
                        last_sent_cmd = next_cmd
                    cmd_idx += 1


            outfile.close()
            if ser_conn is not None:
                print(
                    sendCommand(
                        ser_conn,
                        Command.PWM,
                        0,
                        0,
                    )
                )
                print(sendCommand(ser_conn, Command.DISABLE))

        except KeyboardInterrupt:
            print("\nCanceled system id.")
            if ser_conn is not None:
                msg = sendCommand(ser_conn, Command.DISABLE)

        finally:
            if outfile is not None:
                outfile.close()
