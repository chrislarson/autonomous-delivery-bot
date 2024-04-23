from typing import Any
import serial
import numpy as np
import random
from scipy.signal import chirp, spectrogram
import matplotlib.pyplot as plt
import time
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

    def __init__(self):
        print("Creating chirp object.")

    def generate_trajectory(
        self,
        duration_sec: int,
        dt_sec: float,
    ):

        start = 0
        stop = duration_sec
        num = int((stop - start) / dt_sec)

        t = np.linspace(start, stop, num)
        w = chirp(t, f0=1, f1=0.2, t1=stop, method="linear")
        plt.plot(t, w)
        plt.title("Linear Chirp, f(0)=2, f(10)=0.5")
        plt.xlabel("t (sec)")
        plt.show(block=True)
