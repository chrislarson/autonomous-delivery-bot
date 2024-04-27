import numpy as np
import math


if __name__ == "__main__":

    waypoints = np.array(
        [[0.0, 0.0], [-804.04526774, 9679.08496094], [-87.7140274, 9679.08496094]]
    )

    thetas = np.array(len(waypoints - 1))
    disps = np.zeros(len(waypoints) - 1)

    for i in range(len(waypoints)):
        if i == 1:
            disp = np.linalg.norm(waypoints[i] - waypoints[i - 1])
            disps[i - 1] = disp
            theta = math.atan2(waypoints[i, 1], waypoints[i, 0])
            theta2 = math.acos(waypoints[i, 1] / disp)
            print(theta - (math.pi / 2))
            print(theta2)
            # thetas[i-1] = theta

    # print(thetas)
    # print(disps)
    # diffs = np.diff(waypoints)
    # print(diffs)
