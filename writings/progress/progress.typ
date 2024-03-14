#set page(margin: (top: 1.75cm, bottom: 1.75cm, left: 1.75cm, right: 1.75cm))
#set math.equation(numbering: "(1)")
#set align(center)

#show figure.where(kind: table): set figure.caption(position: top)
#show heading: it => {
  it
  v(0.25em)
}

= MEGN540 Project Progress Report
Sebastian Negrete-Alamillo, Keenan Buckley, Chris Larson

#set heading(numbering: "1.")
#set align(left)

// ====================
// Part 1: Technical
// ====================

= Problem:
// What problem are you solving?
Many people are unable (or unwilling) to get up, walk to, and grab an item they
need or desire around their home. Examples include elderly persons who need to
take medication at a specific time daily but may be forgetful, and college
students who are thirsty but too incapacitated to get their next beverage for
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

== Sensors
// What sensors will you use?
- #strong("Stereo camera:") Luxonis Oak-D Lite for visual odometry, depth
  estimation, and object detection.
- #strong("IMU:") MPU-6050 sensor for pose estimation and motion control feedback.
- #strong("Load sensor:") MPS20N00400 pressure sensor and HX711 amplifier to
  measure payload.
- #strong("Wheel encoder (x2):") Hall encoders for motion control feedback.

== Actuators
// What actuators will you use?
- #strong("DC motor (x2):") 12V, 150rpm DC motors to power the robot's drivetrain.

== PCB
// What will your PCB do?
- #strong("LED Demuxer:") Our circuit board's primary purpose is to enable us to
  indicate our robot's status visually without occupying too many pins on our
  microcontroller. To do so, the PCB will include a 3 to 8 demuxer, a resistor,
  and ports for 8 LEDs. As a second-order effect, the PCB will clean up our
  robot's wiring by supplying 5V power rails.

== Software
- Visual odometry module for depth perception and pose estimation.
- Object detection module for person identification.
- Path planning module for trajectory generation.
- Motion control module to power motors and follow the trajectory.

= System Integration:
// What is your high-level integration plan?
We worked as a team to assemble our robot's chassis and prototype mechatronic
system. We are working in parallel on our robot's software modules, and to
simplify integration, we are using Docker containers to assist with environment
compatibility and a GitHub branch/pull/merge workflow to manage our codebase.
During April, we will resume working as a team to finalize the integration of
our systems and tune the robot.

// ====================
// Part 2: Programmatic
// ====================

= Project Plan:
// What is your project schedule and key milestones?
Our work/deliverables are planned in two-week increments to keep tabs on
progress and enable us to react quickly to issues that arise. We have created a
backlog of deliverables on GitHub and are organizing our work using a Kanban
board. @project_plan shows our milestones and their current status. We have
completed the first three milestones and are progressing on the rest. Completed
works are shown in @completed_works.

#figure(
  stack(
    dir: ltr,
    spacing: 1em,
    image("prototype_build.jpg", height: 22%),
    image("pcb_build.png", height: 22%),
    image("pcb_render.png", height: 22%),
  ),
  placement: auto,
  caption: [Works completed to date. (L) Prototype mechatronic system, (R) PCB design.],
) <completed_works>

#figure(table(
  columns: (auto, auto, auto, auto, auto),
  rows: auto,
  // Header
  [*Milestone*],
  [*Date*],
  [*Status*],
  [*Description*],
  [*Requirements*],
  // Milestone 1
  [1],
  [2024-02-07],
  [\u{2713}],
  [Material acquisition and planning],
  [
    - Bill of Materials created
    - Ordered necessary materials
  ],
  // Milestone 2
  [2],
  [2024-02-21],
  [\u{2713}],
  [Software module design],
  [
    - Project repo instantiated
    - Module specifications created
    - ROS framework installed
  ],
  // Milestone 3
  [3],
  [2024-03-06],
  [\u{2713}],
  [Prototype build and PCB design],
  [
    - PCB design finalized
    - Materials assembled
  ],
  // Milestone 4
  [4],
  [2024-03-20],
  [In Progress],
  [Software implementation],
  [
    - Visual odometry module implemented
    - Object detection module implemented
  ],
  // Milestone 5
  [5],
  [2024-04-10],
  [In Progress],
  [Software implementation],
  [
    - Path planning module implemented
    - Motion control module implemented
  ],
  // Milestone 6
  [6],
  [2024-04-24],
  [To do],
  [Prototype refinement],
  [
    - PCB installed/integrated
    - Path planning and motion control tuned
  ],
  // Milestone 7
  [7],
  [2024-04-30],
  [To do],
  [Deliverables],
  [
    - Project is demonstrated to the class.
    - The project report is submitted.
  ],
), caption: "Project plan and status.") <project_plan>

== Work Split
// How have you split up the work?
- #strong("As a team:")

- #strong("Individually:")

== Critical Paths
// What are your critical paths?

= Budget Estimate
// What is your current budget estimate?
#strong("Budget:") \$300 (\$100 per team member). Our team agreed on this as the
upper limit, based upon initial exploration of components we know we need to
purchase (chassis, Arduino, motors, and motor controllers) and our purchasing
power. We own several more expensive components, including an NVIDIA Jetson Orin
Nano and a Stereo RGB-D camera.

= Risks
== Technical
// What is your highest technical risk, and how are you mitigating that risk?
== Programmatic
// What is your highest programmatic risk, and how are you mitigating that risk?
