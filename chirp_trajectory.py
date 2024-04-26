from typing import Any, Literal, Union, Tuple
import numpy as np
from scipy.signal import chirp
import matplotlib.pyplot as plt
import os
import time

class ChirpTrajectory:

    def generate_trajectory(
        self,
        data_dir: str,
        duration_sec: int,
        dt_sec: float,
        traj_type: Union[Literal["ROTATION"],Literal["LINEAR"]] = "LINEAR",
        start_freq: float = 0.01,
        end_freq: float = 1.0,
        pwm_max: int = 100,
        pwm_deadband: int = 20,
        include_turn_dynamics: bool = False,
        show_plot: bool = True
    ) -> Tuple[str, str]:

        if duration_sec < 5:
            # TODO: Exception
            pass

        # Create a linearly increasing PWM segment to begin the trajectory.
        start_lin = 0
        stop_lin = 1
        num_lin_steps = int((stop_lin - start_lin) / dt_sec)
        ts_lin = np.linspace(start_lin, stop_lin, num_lin_steps)
        pwm_lin = np.interp(ts_lin, [start_lin, stop_lin], [start_lin, stop_lin])

        # Create a frequency sweep (chirp) PWM segment.
        start_fs = 0
        stop_fs = duration_sec
        num_fs_steps = int((stop_fs - start_fs) / dt_sec)
        ts_fs = np.linspace(start_fs, stop_fs, num_fs_steps)
        pwm_fs = chirp(ts_fs, f0=start_freq, f1=end_freq, t1=stop_fs, method="linear")

        # Create a combined (linear + chirp) trajectory.
        ts_fs = ts_fs + stop_lin # Shift all FS times to be after linear seg.
        ts = np.concatenate((ts_lin, ts_fs))
        pwms = np.concatenate((pwm_lin, pwm_fs))

        # Set left and right PWM based on traj type and max pwm
        pwm_left = pwm_max * pwms
        pwm_right =  pwm_left if traj_type=="LINEAR" else -1*pwm_left

        trajectory = np.stack((ts, pwm_left, pwm_right), axis=-1)
        self._trajectory = trajectory

        # Create timestamped directory in data_dir for this trajectory
        traj_dir = os.path.join(data_dir, 'sysid_' + str(int(time.monotonic())))
        os.mkdir(traj_dir)

        # Save trajectory as csv
        csv_path = os.path.join(traj_dir, "chirp_traj_" + traj_type + ".csv")
        np.savetxt(csv_path, trajectory, delimiter=",", fmt="%10.2f")

        # Save trajectory as png
        fig, ax = plt.subplots()
        ax.set_xlabel("Time (s)")
        ax.set_ylabel("PWM")
        plt.step(ts, pwm_left, where="pre")
        plt.step(ts, pwm_right, where="pre")
        ax.axhline(
            y=pwm_deadband,
            color="black",
            linewidth=0.8,
            alpha=0.5,
            ls="--",
            label="+ Deadband",
        )
        ax.axhline(
            y=-pwm_deadband,
            color="black",
            linewidth=0.8,
            alpha=0.5,
            ls="--",
            label="- Deadband",
        )
        ax.set_ylim(ymin=-pwm_max, ymax=pwm_max)
        fig_path = os.path.join(traj_dir, "chirp_traj_" + traj_type + ".png")
        plt.savefig(fig_path)
        if show_plot:
            plt.show()

        return (traj_dir, csv_path)


if __name__ == "__main__":
    data_dir = os.path.join(os.getcwd(), "data")
    chirp_traj = ChirpTrajectory()
    chirp_traj.generate_trajectory(data_dir, 20, 0.02, "LINEAR")
