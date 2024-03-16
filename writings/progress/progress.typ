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

== PCB
// What will your PCB do?

- #emph("LED Demuxer:") Our circuit board's primary purpose is to enable us to
  indicate our robot's status (locating a target, planning, delivery in progress,
  delivered, ready, error, etc.) visually without occupying too many pins on our
  microcontroller. Our PCB will incorporate a 3 to 8 demuxer, a resistor, and
  ports for 8 LEDs to achieve this. The PCB will also clean up our robot's wiring
  by supplying 5V power rails for our sensors. Our initial PCB design is in
  @completed_works.

== Software
- #emph("Visual odometry") module for depth perception and pose estimation.
- #emph("Object detection") module for person identification.
- #emph("Path planning") module for trajectory generation.
- #emph("Motion control") module to power motors and follow the trajectory.

= System Integration:
// What is your high-level integration plan?

We worked as a team to assemble our robot's chassis and prototype mechatronic
system. We are working in parallel on our robot's software modules. To simplify
integration, we use Docker containers to ensure environment compatibility and a
GitHub branch/pull/merge workflow to manage our codebase. During April, we will
work together as a team to first integrate our software modules (running on
Nvidia Jetson) with our mechatronic system (controlled on Arduino Uno), and then
to implement our motion controller. We will assign debugging tasks individually
as necessary. We intend to have a working product by 4/24 and one week to
refine/optimize.

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

We have combined work in areas where we have less experience and assigned
individual work in areas where we have more experience. The work split is as
follows:

- Planning and design (team)
- Prototype build (team)
- PCB design (Keenan)
- Visual odometry (Chris)
- Object/Person detection (Sebastian)
- Path planning (Keenan)
- Final assembly (team)
- Motion controller (team)

== Critical Paths
// What are your critical paths?

Critical Path 1:
- Software module integration. Our first critical path is end-to-end integration
  of our software modules. For our system to function as intended, our data flow
  through sensing, planning, and action phases must be seamless. We are
  incorporating our module interfaces over the next two weeks to avoid any
  last-minute surprises.

Critical Path 2:
- PCB integration and assembly. Our second critical path is printing and
  integrating our PCB into our mechatronic system. We have a prototype mechatronic
  system, but a successful project outcome requires incorporating a custom PCB, so
  we are working diligently to order the PCB as soon as possible. That should give
  us enough time to re-order if necessary.

= Budget Estimate
// What is your current budget estimate?

We aim to spend \$300 or less (\$100 per team member) on parts. To date, we have
spent \$254 acquiring our chassis, motors, Arduino, motor controller, voltage
regulators, batteries, and miscellaneous necessities (wiring, acrylic platforms,
tape, etc.). The most expensive components we had to purchase were 4S LiPo
batteries and a compatible charger to power the Nvidia Jetson and our motors.

The remaining items to be purchased include the PCB parts: the prints, the 3:8
demuxers, and the LEDs.

= Risks
== Technical
// What is your highest technical risk, and how are you mitigating that risk?

- \#1 - Jetson platform coding: NVIDIA drivers and SDKs
- \#2 - Real-time processing and control speed
- \#3 - Sensor fidelity (cameras in a relatively visually uniform room)

Our highest technical risk is deploying our software modules on the Nvidia
Jetson. Given Nvidia has unique hardware and proprietary software integration,
there is a risk that as we develop code individually on our machines, it simply
won't work on the Jetson with Nvidia SDKs. To mitigate this risk, we compiled
Docker images on the Jetson that contain a replica of its environment. We will
launch containers from these images for development environments while coding on
our machines. We feel this approach will mitigate most risks. Still, to ensure
we have our eyes on this issue, we plan to deploy our codebase on the Jetson
frequently through the remainder of the semester to ensure no hidden issues show
up while changing our code.

== Programmatic
// What is your highest programmatic risk, and how are you mitigating that risk?

- \#1 - PCB delivery timeline
- \#2 - Bandwidth constraints over Spring Break

We believe our highest programmatic risk is the PCB delivery timeline. Given our
inexperience with PCBs, and our need to solder a small, surface-mounted demuxer
to it, we feel that the two-week delivery time will cause issues if we don't
approach it carefully. We designed our PCB this week to mitigate this risk and
intend to order it before Spring Break, which will leave us enough time to and
order a second PCB if something goes wrong.