# Send CMD, PWM_L, PWM_R,
from typing import Any
import serial
import numpy as np
import random
import matplotlib.pyplot as plt
import time
import os


class StepTrajectory:
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

    # Generates the step trajectory.
    # Note: Initial implementation sets pwm_l and pwm_r to equivalent values,
    # which does not include turning dynamics.
    def generate_trajectory(self, test_dir: str) -> np.ndarray:
        pwm_signs = [-1, 1]
        pwm_values = (
            np.arange(self._pwm_deadband, self._pwm_max)
            if self._exclude_deadband
            else np.arange(0, self._pwm_max)
        )

        cmd_cnt = int(self._duration_sec / self._dt_sec)
        dt_multipliers = np.array([4, 8, 16, 32])
        cmd_durations = self._dt_sec * dt_multipliers

        pwm_left = np.zeros(cmd_cnt + 1)
        pwm_right = np.zeros(cmd_cnt + 1)
        times = np.arange(0, self._duration_sec + self._dt_sec, self._dt_sec)

        i = 0
        while i < cmd_cnt:
            cmd_duration = np.random.choice(cmd_durations)
            cmd_steps = int(cmd_duration / self._dt_sec)
            cmd_steps = min(cmd_steps, cmd_cnt - i)
            pwm_sign = random.choice(pwm_signs)
            pwm_l = np.random.choice(pwm_values) * pwm_sign
            if self._include_turn_dynamics:
                pwm_r = np.random.choice(pwm_values) * pwm_sign
            else:
                pwm_r = pwm_l
            for n in range(cmd_steps):
                pwm_left[i + n] = pwm_l
                pwm_right[i + n] = pwm_r
            i += cmd_steps

        traj = np.stack((times, pwm_left, pwm_right), axis=-1)
        self._trajectory = traj
        csv_path = os.path.join(test_dir, "step_traj.csv")
        np.savetxt(csv_path, traj, delimiter=",", fmt="%10.2f")
        return traj

    # Plots the step trajectory.
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
            fig_path = os.path.join(test_dir, "step_trajectory.png")
            plt.savefig(fig_path)
        else:
            plt.show()


if __name__ == "__main__":

    # 0. Create a directory to store data.
    test_id = "sysid_" + str(int(time.monotonic()))
    test_dir = os.path.join("data", test_id)
    os.mkdir(test_dir)

    # 1. Generate step trajectory.
    TEST_DURATION_SEC = 20
    DT_SEC = 0.02
    step_traj = StepTrajectory(test_dir, TEST_DURATION_SEC, DT_SEC, 100, 20, True, True)
    traj = step_traj.generate_trajectory(test_dir)
    step_traj.plot_trajectory(test_dir, True)

    # 2. Connect to serial.
    # ser = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
    # ser.reset_input_buffer()

    # 3. Iterate through trajectory
    # -> Reads in received lines on every iteration.
    # -> Sends new PWM commands when command has changed (new PMW != prev PWM)
    sys_id_in_progress = True

    cmd_idx = 0
    next_cmd = traj[cmd_idx, :]
    start_time = time.monotonic_ns()
    last_sent_cmd = None

    while sys_id_in_progress:

        # 1. Read in serial & write to output file.
        # TODO

        cmd_time = next_cmd[0]
        dt_ns = time.monotonic_ns() - start_time
        if dt_ns >= cmd_time * 1_000_000_000:
            if (
                last_sent_cmd is None
                or last_sent_cmd[1] != next_cmd[1]
                or last_sent_cmd[2] != next_cmd[2]
            ):
                # 2. Send via serial.
                print(next_cmd)  # TODO: Send via serial here.
                last_sent_cmd = next_cmd

            cmd_idx += 1
            if cmd_idx >= len(traj):
                sys_id_in_progress = False
            else:
                next_cmd = traj[cmd_idx, :]

    # 3. Final read from serial and close out program.
