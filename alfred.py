import cmd
import os
import serial
import time
from typing import Union

from chirp_trajectory import ChirpTrajectory
from sysid import SystemID
from arduino.serialio import enableArduino, genCmd, sendCommand, Command


class Aifr3dCLI(cmd.Cmd):
    prompt: str = "Aifr3d>> "
    intro: str = 'Welcome to AIFR3D. Type "help" for available commands.'

    _session_directory: str

    _ser_port: str = "/dev/ttyAMC0"
    _ser_port2: str = "/dev/tty.usbmodem2101"
    _ser_baud: int = 115200

    _serial: Union[serial.Serial, None] = None
    _connected: bool = False

    _sysid_traj: Union[str, None] = None

    def __init__(self):
        super().__init__()

        cwd = os.getcwd()
        data_dir = "data"
        session_id = str(int(time.monotonic()))
        session_dir = os.path.join(cwd, data_dir, session_id)
        os.makedirs(session_dir)
        self._session_directory = session_dir

        print("------------------------------------")
        print("** Session ID:", session_id)
        print("** Session Directory:", session_dir)
        try:
            ser = None
            ser_port: str = ""
            try:
                ser = serial.Serial(self._ser_port, self._ser_baud, timeout=1)
                ser_port = self._ser_port
            except serial.SerialException:
                ser = serial.Serial(self._ser_port2, self._ser_baud, timeout=1)
                ser_port = self._ser_port2
            except Exception as e:
                print(e)

            if ser is not None:
                ser.reset_input_buffer()
                ser.read_all()
                self._serial = ser
                self._connected = True
                print()
                print(
                    "** // CONNECTED! // to robot on serial port {} with BAUD {}!".format(
                        ser_port, self._ser_baud
                    )
                )
        except:
            print("** // NOT CONNECTED! // to robot. Running in disconnected mode.")
        print("------------------------------------")
        print("")

    def do_gen_sysid_trajectory(self, line):
        """
        Generates a frequency sweep (chirp) trajectory.

        Usage:
            gen_sysid_trajectory [duration_sec] [dt_sec] *[traj_type (^linear OR rotation)]

        * - optional parameter.
        ^ - optional parameter default value.
        """
        args = line.split(" ")
        try:
            duration_sec = int(args[0])
            dt_sec = float(args[1])
            traj_type = "LINEAR" if len(args) < 3 else str(args[2])
            if traj_type[0].upper() == "L":
                traj_type = "LINEAR"
            else:
                traj_type = "ROTATION"
            chirp_traj = ChirpTrajectory()
            dir, csv_path = chirp_traj.generate_trajectory(
                self._session_directory,
                duration_sec,
                dt_sec,
                traj_type=traj_type,
                show_plot=False,
            )
            self._sysid_traj = csv_path
            print("SystemID trajectory generated and saved in", dir)
        except Exception as e:
            print(e)
            print("Something went wrong. Please try again!")

    def do_id_system(self, line):
        """
        Usage:

            id_system *[traj_path(csv)]

        * - optional parameter.
        """
        try:
            args = line.split(" ")
            traj_path = str(args[0])
            if len(traj_path) < 1:
                print(
                    "No path to trajectory .csv provided. Attempting to use last trajectory generated during session..."
                )
                if self._sysid_traj is None:
                    print(
                        "No trajectory generated during this session. Generating one..."
                    )
                    self.do_gen_sysid_trajectory("20 0.02")
                    traj_path = str(self._sysid_traj)
                else:
                    traj_path = self._sysid_traj

            print("Running system identification using trajectory in", traj_path)

            sysid = SystemID()
            if self._serial is not None:
                print("Connected to robot. Running live system ID.")
            else:
                print("Disconnected from robot. Simulating system ID.")

            sysid.id_sys(traj_path, self._serial)

        except Exception as e:
            print(e)
            print("Something went wrong! Please try again.")

    def do_drive(self, line):
        """
        Sends drive command to the robot (if connected) else prints the drive command to console.

        Usage:
            drive [mode (displacement OR velocity)] [linear_value] [angular_value]
        """
        try:
            args = line.split(" ")
            arg_mode = str(args[0])
            arg_linear_val = float(args[1])
            arg_angular_val = float(args[2])

            if self._serial is not None:
                # Connected, send to robot.
                self._serial.reset_input_buffer()
                enableArduino(self._serial)
                msg = sendCommand(
                    self._serial, Command.WAYPOINT, arg_angular_val, arg_linear_val
                )
                # print(msg)
            else:
                # Not connected, print to console.
                print(arg_mode, arg_linear_val, arg_angular_val)
        except Exception as e:
            print(e)
            print("Something went wrong. Please try again!")

    # def do_find_targets(self, line):
    #     """Locates person targets on the Oak-D Lite and saves their 3D world coordinates."""
    #     files_and_dirs = os.listdir(self.current_directory)
    #     for item in files_and_dirs:
    #         print(item)

    # def do_deliver(self, line):
    #     """List files and directories in the current directory."""
    #     files_and_dirs = os.listdir(self.current_directory)
    #     for item in files_and_dirs:
    #         print(item)

    def do_enable(self, line):
        """
        If connected to robot, send an Enable command over serial.
        """
        if self._serial is not None:
            msg = sendCommand(self._serial, Command.ENABLE)
            print("Enabled robot.")
        else:
            print("Not connected to robot. Cannot enable.")

    def do_disable(self, line):
        """
        If connected to robot, send a Disable command over serial.
        """
        if self._serial is not None:
            msg = sendCommand(self._serial, Command.DISABLE)
            print("Disabled robot.")
        else:
            print("Not connected to robot. Cannot disable.")

    def do_quit(self, line):
        """
        Exit the CLI.
        If connected to robot, it will send a Disable command.
        """
        if self._serial is not None:
            self.do_disable("")
        return True


    def is_connected(self):
        if self._serial is not None:
            return self._serial.is_open
        return False

    def postcmd(self, stop, line):
        print()  # Add an empty line for better readability
        return stop

if __name__ == "__main__":
    alfred = Aifr3dCLI()
    try:
        alfred.cmdloop()
    except KeyboardInterrupt:
        if alfred.is_connected():
            alfred.do_disable("")
            print("Exiting.")
