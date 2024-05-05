// Drawings and Pseudocode:

//  - path planning     -> sebastian
//  - foam prototype images -> sebastian

== Translate Waypoints to Commands
After determining the waypoints, these were then converted to linear distance in
millimeters and angular displacement in radians to be sent to the Arduino. 
To determine the depth and angle, we looped over the waypoints found in the trapezoidal trajectory and found the difference between them. From there, were took the Euclidean distance to determine the linear displacement between the current and the next waypoint. We found that our camera was often providing us with a depth measurement, or z-coordinate, that was greater than what we expected, thus, we reduced the displacement by 20% by multiplying the resulting displacement by 0.8. This corrected both our displacement error as well as augmented the turn radius between waypoints to be over 
the deadband and increase turn accuracy on carpeted surfaces.

To determine the turn radius, we used the equation below, where we centered our
turns such that the origin was straight ahead at the robot's $pi/2$ axis and
corrected the angles by subtracting the previous heading.

#show math.equation: set text(font: "Fira Math")
$ op(#sym.theta)_op("Calc") = op("atan2")(op("Diff_z"), op("Diff_x")) $
$ op(#sym.theta)_op("Center") = op(#sym.theta)_op("Calc") - pi/2 $
$ op(#sym.theta)_op("Correct") = op(#sym.theta)_op("Center") - op(#sym.theta)_op("Prev") $

From there, the Arduino was fed a turn command with the calculated angle first, then a displacement command. 

== Prototyping

