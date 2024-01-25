#set page(margin: (top: 1.75cm, bottom: 1.75cm, left: 1.75cm, right: 1.75cm))
#set math.equation(numbering: "(1)")
#set align(center)
#show figure.where(kind: table): set figure.caption(position: top)

= MEGN540 Project Proposal: Delivery Rover
Sebastian Negrete-Alamillo, Keenan Buckley, Chris Larson

#set heading(numbering: "1.")
#set align(left)

= Concept and Requirements
We will build a mobile delivery robot. Upon activation, the robot will identify the person nearest to it, drive to that person, and deliver a payload. The system must have the ability to:

1. Listen for and react to an activation signal.
2. Identify persons in its FOV and target the nearest person to it (if any).
2. Drive to that person in a straight line on a flat, carpeted surface.
3. Stop upon arriving at the person and deliver the payload.

= Budget and Plan
Budget: \$300 (\$100 per team member).

#figure(table(
  columns: (auto, auto, auto, auto),
  rows: auto,
  // Header
  [*Milestone*],
  [*Date*],
  [*Description*],
  [*Requirements*],
  // Milestone 1
  [1],
  [2024-02-07],
  [Material acquisition and planning],
  [
    ...
  ],
  // Milestone 2
  [2],
  [2024-02-21],
  [Prototype build],
  [
  ...
  ],
  // Milestone 3
  [3],
  [2024-03-06],
  [...],
  [...],
  // Milestone 4
  [4],
  [2024-03-20],
  [...],
  [...],
  // Milestone 5
  [5],
  [2024-04-10],
  [...],
  [...],
  // Milestone 6
  [6],
  [2024-04-24],
  [...],
  [...],
  // Milestone 7
  [7],
  [2024-04-30],
  [Demonstration],
  [
    - Project is demonstrated to the class.
  ],
  // Milestone 8
  [8],
  [2024-05-05],
  [Report],
  [
    - Project report is finalized and submitted.
  ],
), caption: "Initial project plan.") <project_plan>

= System Components

== Sensors
- Stereo camera (RGB-D) for visual odometry, depth estimation, and object detection.
- Wheel encoders for motion control.

== Affectors
- DC motors to rotate wheels and drive the robot.

== Software
- Visual odometry module for depth perception and pose estimation.
- Object detection module for person identification.
- Path planning module for trajectory generation.
- Motion control module to power motors and follow trajectory.

== PCB
- Require PCB Development

== Prototype
- Result in a functional prototype
