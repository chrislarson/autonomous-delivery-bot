import matplotlib.pyplot as plt
import numpy as np
from roboticstoolbox.tools.trajectory import Trajectory, mtraj, mstraj, trapezoidal
from spatialmath import SO2

plt.style.use("_mpl-gallery")

via = SO2(30, unit="deg") * np.array([[0, 0, -3], [0, 1, 10]])

print(via.T)

traj0 = mstraj(via.T, dt=0.2, tacc=1, qdmax=[2, 1])

traj0.plot(block=True)
# traj0.qd

via = np.array([[0, 0, -3], [0, 1, 10]])

# traj: Trajectory = mtraj(trapezoidal, [0, 0], [-10, 10], 50)

# traj_wp = mstraj([[0, 0], [0, 1], [-10, 10]], dt=0.2, tacc=2)

# make data
x = traj0.q[:, 0]
y = traj0.q[:, 1]

# xd = traj0.qd[:, 0]
# yd = traj0.qd[:, 1]

# plot
fig, ax = plt.subplots()

ax.plot(x, y, linewidth=2.0)

# ax.set(xlim=(0, 8), xticks=np.arange(1, 8), ylim=(0, 8), yticks=np.arange(1, 8))

# plt.show()
