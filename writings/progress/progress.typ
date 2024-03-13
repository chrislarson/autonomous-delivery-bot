#set page(margin: (top: 1.75cm, bottom: 1.75cm, left: 1.75cm, right: 1.75cm))
#set math.equation(numbering: "(1)")
#set align(center)
#show figure.where(kind: table): set figure.caption(position: top)

= MEGN540 Project Progress Report: Delivery Rover
Sebastian Negrete-Alamillo, Keenan Buckley, Chris Larson

#set heading(numbering: "1.")
#set align(left)

// Part 1: Technical

= Problem Statement
// What problem are you solving?

= Design Concept
// What is your design concept?

We are building a mobile delivery robot. Upon activation, the robot will identify
the person nearest to it, drive to that person, and deliver a payload. Many use
cases exist for such a robot, including beverage and medication delivery. The
robot's capabilities are relevant in industry, so building it will be a good
experience for us. The system must have the ability to:

1. Listen for and react to an activation signal.
2. Identify persons in its FOV and target the nearest person to it (if any).
2. Drive to that person in a straight line on a flat, carpeted surface.
3. Stop upon arriving at the person and deliver the payload.

== Sensors and Actuators
// What Sensors and Actuators will you use?

== Custom PCB 
// What will your PCB do?

= Integration Plan 
// What is your high-level integration plan?


// Part 2: Programmatic

= Schedule and Milestones
// What is your project schedule and key milestones?
#strong("Initial Plan:") We've planned our work/deliverables in two-week
increments to keep tabs on progress and enable us to react quickly to any
issues. @project_plan shows our milestones and current status. We have completed the first three, and are progressing on the remaining. 

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

#figure(stack(dir: ltr, spacing: 1em, image("prototype_build.jpg", height: 22%), image("pcb_build.png", height: 22%), image("pcb_render.png", height: 22%)), placement: auto, caption: [Completed works. (L) Prototype mechatronic system, (R) PCB design.])



== Work Split 
// How have you split up the work?

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
// What is your highest technical risk and how are you mitigating that risk?
== Programmatic 
// What is your highest programmatic risk and how are you mitigating that risk?



= System Components

== Sensing and Affecting

- #strong("Sensors:") Stereo camera (RGB-D) for visual odometry, depth estimation,
  and object detection. Wheel encoders for motion control.

- #strong("Affectors:") DC motors to power the robot's drivetrain. Servos to
  actuate the payload delivery mechanism.

== Software
- Visual odometry module for depth perception and pose estimation.
- Object detection module for person identification.
- Path planning module for trajectory generation.
- Motion control module to power motors and follow the trajectory.

== PCB
We will create a PCB with LEDs to indicate the system's state (powered on,
identifying recipients, planning the path, and making the delivery). This PCB
will allow us to build something useful for the robot while not greatly
exceeding our current abilities.

= Prototyping
All three team members are in Mechatronics and SLAM, and we have experience with
all system components. We view this project as an opportunity to "put it all
together" and feel confident in our ability to build a functioning prototype.