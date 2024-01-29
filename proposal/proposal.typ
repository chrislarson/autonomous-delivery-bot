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
#strong("Budget:") \$300 (\$100 per team member).

#strong("Initial Plan:") We will plan our work/deliverables in two-week increments to keep tabs on progress and enable us to react quickly to any issues that arise. @project_plan shows a first-pass plan describing these increments.

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
    - Bill of Materials created
  ],
  // Milestone 2
  [2],
  [2024-02-21],
  [Software module design],
  [
  ...
  ],
  // Milestone 3
  [3],
  [2024-03-06],
  [Prototype build and PCB design],
  [...],
  // Milestone 4
  [4],
  [2024-03-20],
  [Software implementation],
  [...],
  // Milestone 5
  [5],
  [2024-04-10],
  [Software implementation],
  [...],
  // Milestone 6
  [6],
  [2024-04-24],
  [Prototype refinement],
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
    - The project report is finalized and submitted.
  ],
), caption: "Initial project plan.") <project_plan>

= System Components

== Sensing and Affecting

- #strong("Sensors:") Stereo camera (RGB-D) for visual odometry, depth estimation, and object detection. Wheel encoders for motion control.

- #strong("Affectors:") DC motors to power the robot's drivetrain. Servos to actuate the payload delivery mechanism.

== Software
- Visual odometry module for depth perception and pose estimation.
- Object detection module for person identification.
- Path planning module for trajectory generation.
- Motion control module to power motors and follow the trajectory.

== PCB
- We will 

= Prototyping
- Result in a functional prototype
