// Drawings and Pseudocode:

//  - path planning     -> sebastian
//  - foam prototype images -> sebastian

== Path planning
After determining the waypoints, these were then converted to linear distance in
millimeters and angular displacement in radians to be sent to the Arduino. We
looped over the

To determine the turn radius, we used the equation below, where we centered our
turns such that the origin was straight ahead at the robot's $pi/2$ axis and
corrected the angles by subtracting the previous heading.

#show math.equation: set text(font: "Fira Math")
$ op(#sym.theta)_op("Calc") = op("atan2")(op("Diff_z"), op("Diff_x")) $
$ op(#sym.theta)_op("Center") = op(#sym.theta)_op("Calc") - pi/2 $
$ op(#sym.theta)_op("Correct") = op(#sym.theta)_op("Center") - op(#sym.theta)_op("Prev") $

Since we found that are detection was often providing us with a Euclidean
distance that was greater than what we expected, we reduced the depth
measurement by 20% by multiplying it by 0.8.

== Prototyping

