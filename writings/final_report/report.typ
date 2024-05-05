#set page(margin: (top: 1.75cm, bottom: 1.75cm, left: 1.75cm, right: 1.75cm))
#set math.equation(numbering: "(1)")
#set align(center)

#show figure.where(kind: table): set figure.caption(position: top)
#show heading: it => {
  it
  v(0.25em)
}

= MEGN540 Project Final Report
Sebastian Negrete-Alamillo, Keenan Buckley, Chris Larson

#set heading(numbering: "1.")
#set align(left)

// ====================
// Part 1: Technical
// ====================

= Problem:
// What problem are you solving?

Many people are unable or unwilling to get up, walk to, and grab an item they
need or desire around their home. Examples include elderly persons who need to
take medication at a specific time daily but may be forgetful, and college
students who are thirsty but too incapacitated to get their next beverage
themselves.

We are designing and building a product that can deliver a necessary item to
those people when they need it.

= Design Concept:
// What is your design concept?

We are building a two-platform tank-drive mobile delivery robot. The lower
platform will secure our electronics, and the upper platform will carry the
delivery payload. Upon activation, our robot will identify the person nearest
it, drive to that person, and deliver the payload. Our robot will have the
ability to:

1. Listen for and react to an activation signal. To begin, this will be a serial
  command issued via SSH.
2. Identify persons in its FOV and target the nearest person to it (if any).
3. Orient itself toward the person and drive to them in a straight line on a flat,
  carpeted surface.
4. Carry a payload of at least 16oz.
5. Stop within arm's reach of the target person to deliver the payload.

// PCB Description:
//  - Keenan

// Requirements:
//  - from prev work

// Drawings and Pseudocode:
//  - person detection  -> chris
//  - path planning     -> sebastian
//  - motion control    -> chris
//  - serial            -> keenan
//  - foam prototype images -> sebastian

== Sensors
// What sensors will you use?
- #emph("Stereo camera:") Luxonis Oak-D Lite for visual odometry, depth
  estimation, and object detection.
- #emph("IMU:") MPU-6050 for pose estimation and motion control feedback.
- #emph("Load sensor:") MPS20N00400 sensor and HX711 amplifier to measure payload.
- #emph("Wheel encoder (x2):") Hall encoders for motion control feedback.

== Actuators
// What actuators will you use?
- #emph("DC motor (x2):") 12V, 150rpm DC motors to power the robot's drivetrain.
