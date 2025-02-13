import cmd
import os
import serial
import time
from typing import Union, List, Any

from chirp_trajectory import ChirpTrajectory
from system_id import SystemID
from detect_persons import DetectPersons
from arduino.serialio import enableArduino, genCmd, sendCommand, Command, LED


class Aifr3dCLI(cmd.Cmd):
    prompt: str = "Aifr3d>> "
    intro: str = 'Welcome to AIFR3D. Type "help" for available commands.'

    _session_directory: str

    _ser_port: str = "/dev/ttyACM0"
    _ser_port2: str = "/dev/tty.usbmodem1101"
    _ser_baud: int = 115200

    _serial: Union[serial.Serial, None] = None
    _connected: bool = False

    _sysid_traj: Union[str, None] = None
    _traj_waypoints: Any
    _traj_thetas: List[float] = []
    _traj_disps: List[float] = []

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
                    "** // CONNECTED! // to robot on serial port {} with BAUD {} :)".format(
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
                    self._serial,
                    Command.DISP,
                    arg_linear_val,
                    arg_angular_val,
                )
                # print(msg)
            else:
                # Not connected, print to console.
                print(arg_mode, arg_linear_val, arg_angular_val)
        except Exception as e:
            print(e)
            print("Something went wrong. Please try again!")

    def do_find_targets(self, line):
        """
        Locates person targets on the Oak-D Lite and saves their 3D world coordinates.

        Usage:
            find_targets [timeout_sec] [show_preview]

        * - optional parameter
        ^ - optional parameter default value
        """

        try:
            if self.is_connected():
                print("Connected to robot. Sending commands for trajectory.")
                enableArduino(self._serial)
                msg = sendCommand(self._serial, Command.STATUS, LED.FIND_TARGETS.value)

            else:
                print("Not connect to robot. Printing trajectory commands.")

            args = line.split(" ")
            arg_timeout = int(args[0])
            arg_show_prev = bool(args[1])
            timeout = arg_timeout if arg_timeout > 0 else 15

            detector = DetectPersons()
            waypoints, thetas, disps = detector.detect(
                self._session_directory, timeout, arg_show_prev
            )
            if waypoints is not None:
                # Found targets.
                self._traj_waypoints = waypoints
                self._traj_thetas = thetas
                self._traj_disps = disps
        except Exception as e:
            print(e)
            print("Something went wrong. Please try again!")
        finally:
            if self.is_connected():
                msg = sendCommand(self._serial, Command.STATUS, LED.READY.value)

    def do_deliver(self, line):
        """
        Sends a trajectory (a series of rotation and displacement commands) to the robot if connected. If not connected,
        prints them.

        Usage:
            deliver
        """

        if self.is_connected():
            print("Connected to robot. Sending commands for trajectory.")
            enableArduino(self._serial)
            # Send LED status 5
        else:
            print("Not connect to robot. Printing trajectory commands.")

        num_rots = len(self._traj_thetas)
        num_disps = len(self._traj_disps)
        traj_length = num_rots + num_disps

        if traj_length > 0:

            msg_traj = (
                sendCommand(self._serial, Command.TRAJ_START, traj_length)
                if self.is_connected()
                else "Start trajectory of length {} steps".format(traj_length)
            )
            print("Trajectory message:", msg_traj)
            for i in range(min(num_rots, num_disps)):
                # First send rotation.
                msg_rot = (
                    sendCommand(self._serial, Command.DISP, 0, self._traj_thetas[i])
                    if self.is_connected()
                    else "Rotation of {} rads".format(self._traj_thetas[i])
                )
                print("Rotation message:", msg_rot)
                # Then send displacement.
                msg_disp = (
                    sendCommand(self._serial, Command.DISP, self._traj_disps[i], 0)
                    if self.is_connected()
                    else "Displacement of {} mm".format(self._traj_disps[i])
                )
                print("Displacement message:", msg_disp)
        else:
            print(
                "Trajectory has zero rotations and displacements. Try finding targets before making a delivery."
            )

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
