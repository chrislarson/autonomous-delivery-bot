#set page(margin: (top: 1.75cm, bottom: 1.75cm, left: 1.75cm, right: 1.75cm))
#set math.equation(numbering: "(1)")

#set heading(numbering: "1.")
#set align(left)

== PCB
// What will your PCB do?

- #emph("LED Demux:") Our circuit board's primary purpose is to enable us to
  indicate our robot's status (Error, Ready, Linear Closed-loop, PWM, Find Targets
  Velocity, Angular Closed-loop, and Waiting) visually, while occupying only 4
  output pins on our microcontroller. Our PCB incorporated a 3 to 8 demuxer, a
  resistor, and ports for 8 LEDs to achieve this. The final PCB directed 4 inputs
  from the arduino to A0, A1, A3, and OE0 on the demux. LE and OE1 were also
  required to be grounded to logical low for the demux to work correctly.

  #figure(
    image("pcb_truth_table.png", height: 22%),
    placement: auto,
    caption: [Truth table for the demux on our PCB. Inputs from A0-A2 set which output Y0-Y7
      to set.],
  )

- #emph("VCC and GND Rails:") The PCB also cleaned up our robot's wiring by
  supplying 5V power rails for our arduino and motor encoders. These power rails
  were 8 pin headers connected together by 17 mil traces. Our trace width of 17
  mil we determined using an online calculator. We set the width so that a 1.4 mil
  thick copper trace would have a 10 degrees farenheit increase in temperature
  when conducting 1 Amp at 70 degrees farenheit. 1 Amp is sufficient to handle the
  Arduino's max current of 1 Amp, in addition to the comparitavly small current
  requirements of the demux and encoders.

  // sources
  // https://www.4pcb.com/trace-width-calculator.html
  // https://docs.oshpark.com/services/two-layer/

  #figure(
    image("pcb_schematic.png", height: 22%),
    placement: auto,
    caption: [PCB schematic from KiCad],
  )

  #figure(stack(
    dir: ltr,
    spacing: 1em,
    image("pcb_build.png", height: 22%),
    image("pcb_render.png", height: 22%),
    image("pcb_result.png", height: 22%),
  ), placement: auto, caption: [PCB design, render, and final result])

== Serial Communication

We designed and implemented our own two way serial communication protocal, using
Python's struct library as a basis. With struct, we defined 11 message types
where each message would begin with a byte that specified which message type was
being sent, and end with the \n character. Python and Arduino's serial libraries
already handled buffering for us, so for both we would read a line of bytes from
serial, and use Python's struct and C++'s packed structs to decode the sequence
of bytes into bytes, ints, and floats. Then we can define specific behavior for
each message on a switch statement for the first byte in each message.