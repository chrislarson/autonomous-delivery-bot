import numpy as np
from typing import Any

_MATCH_DISTANCE = 250  # mm


class Person:
    match_count: int
    coordinates: np.ndarray[Any]
    ma_coord: np.ndarray[float]

    def __init__(self, coord: np.ndarray[float]):
        self.match_count = 0
        self.coordinates = np.array([coord])
        self.ma_coord = coord

    def add_match(self, coord: np.ndarray[float]):
        # Add the coordinate to list of coordinates.
        self.coordinates = np.append(self.coordinates, [coord], axis=0)

        # Increment match count.
        self.match_count = self.match_count + 1

        # Compute new moving average. If there are more than 3 coordinates, use last 3. Else use all coordinates.
        window = min(len(self.coordinates), 3)
        ma = np.ma.average(self.coordinates[:window], axis=0, keepdims=True)
        self.ma_coord = ma

    def check_match(self, coord: np.ndarray[Any]):
        # Check distance between coord and ma_coord.
        distance = np.linalg.norm(self.ma_coord - coord)
        if distance < _MATCH_DISTANCE:
            return True
        return False
