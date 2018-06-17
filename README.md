# nightlighting
This project is meant to control a (battery powered) light so it is only lid in the dark.

We use a small microcontroller (digispark/at tiny 85) for this very simple problem to have some easier to tune knobs than with an analog circuit.

Parts:
  * AT Tiny 85 (Digispark or clone)
  * 1-3 N-Channel Logic level fets for switching the lights (AO3400 or IRLML2502 or whatever you have)
  * LDR - light dependent resistor with any value
  * A resistor of approximately the value you measure at your LDR at the threshold between dark and light. Everything in the range 0.1 .. 10 times that value works perfectly fine
  * optionally a resistor divider of ~10:1 (10k and 100k) to measure the battery voltage

# Software
The software should ...
  * read LDR and filter to get a time constant of at least some seconds
  * evaluate the filtered value against a threshold (with hysteresis) to evaluate dark/bright
  * switch (or optionally PWM) the lights depending on the evaluated situation
  * optionally read the battery voltage, also filtered for at least a second, to disable/darken/whatever the lights on an empty battery

# Prototype
For my prototype, I could reuse some PCBs I had lying around as leftovers from an old WS2801 project.
It acts as a base PCB for all parts, the digispark is connected via wires.
![img/prototype.jpg](Prototype)
