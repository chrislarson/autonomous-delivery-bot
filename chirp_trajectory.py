from typing import Any
import numpy as np
from scipy.signal import chirp
import matplotlib.pyplot as plt
import os


class ChirpTrajectory:
    _test_dir: str
    _duration_sec: int
    _dt_sec: int
    _pwm_max: int
    _pwm_deadband: int
    _exclude_deadband: bool
    _include_turn_dynamics: bool
    _trajectory: np.ndarray[Any]

    def __init__(
        self,
        test_dir: str,
        duration_sec: int,
        dt_sec: float,
        pwm_max: int = 100,
        pwm_deadband: int = 20,
        include_turn_dynamics=True,
        exclude_deadband: bool = True,
    ):
        self._test_dir = test_dir
        self._duration_sec = duration_sec
        self._dt_sec = dt_sec
        self._pwm_max = pwm_max
        self._pwm_deadband = pwm_deadband
        self._exclude_deadband = exclude_deadband
        self._include_turn_dynamics = include_turn_dynamics
        self._trajectory = None

    def generate_trajectory(
        self,
        test_dir,
        duration_sec: int,
        dt_sec: float,
        start_freq: float = 0.01,
        end_freq: float = 1.0,
    ):

        # Frequency sweep segment
        start = 0
        stop = duration_sec
        num = int((stop - start) / dt_sec)

        t = np.linspace(start, stop, num)
        w = chirp(t, f0=start_freq, f1=end_freq, t1=stop, method="linear")

        # Linear increase segment
        start1 = 0
        stop1 = 1
        num1 = int((stop1 - start1) / dt_sec)
        lint = np.linspace(start1, stop1, num1)

        lin = np.interp(lint, [start1, stop1], [start1, stop1])

        t = stop1 + t
        t = np.concatenate((lint, t))
        w = np.concatenate((lin, w))

        pwm_left = self._pwm_max * w
        pwm_right = pwm_left
        traj = np.stack((t, pwm_left, pwm_right), axis=-1)
        self._trajectory = traj
        csv_path = os.path.join(test_dir, "chirp_traj.csv")
        np.savetxt(csv_path, traj, delimiter=",", fmt="%10.2f")
        return traj

    def plot_trajectory(self, test_dir: str, save_fig=True):
        if self._trajectory is None:
            self.generate_trajectory()

        fig, ax = plt.subplots()

        ax.set_xlabel("Time (s)")
        ax.set_ylabel("PWM")

        times = self._trajectory[:, 0]
        pwm_left = self._trajectory[:, 1]
        pwm_right = self._trajectory[:, 2]

        plt.step(times, pwm_left, where="pre")
        plt.step(times, pwm_right, where="pre")
        ax.axhline(
            y=self._pwm_deadband,
            color="black",
            linewidth=0.8,
            alpha=0.5,
            ls="--",
            label="+ Deadband",
        )
        ax.axhline(
            y=-self._pwm_deadband,
            color="black",
            linewidth=0.8,
            alpha=0.5,
            ls="--",
            label="- Deadband",
        )
        ax.set_ylim(ymin=-self._pwm_max, ymax=self._pwm_max)
        if save_fig:
            fig_path = os.path.join(test_dir, "chirp_trajectory.png")
            plt.savefig(fig_path)
        else:
            plt.show()


if __name__ == "__main__":
    chirp_traj = ChirpTrajectory()
    chirp_traj.generate_trajectory(10, 0.02)
