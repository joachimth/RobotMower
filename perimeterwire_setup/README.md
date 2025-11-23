#Perimeter hardware and Code.

The Hardware is:
ESP32
Motor driver, eg. L289N driver, or something newer.

The Perimeter wire hardware will start as being just that.
Hardware for controlling the perimeter wire, with the ability to be switched on/off and monitored via an web API, served by the ESP32.

The robotmower then has the ability to call the perimeter wire hardware and ask for the wire to be turned on or off.
This shall of course be implemented in the robotmower's code also.


Expected perimeter wire details:

The motor driver is driven by 3.2 Khz (two pulse widths 4808 Hz and 2404 Hz).
We use a motor driver with integrated current limiting and thermal switch-off (e.g. MC33926).

The motor driver output voltage can be changed between 6.5-12V via the potentiometer on the DC/DC converter. Adjust the potentiometer so that not more than 1 Ampere current flows.



For the Robotmower:
For receiving the signal, we use a coil (100 mH or 150 mH) in upright position (centered at front in robot) connected to an LM386 operational amplifier (to amplify the received signal).
When using the LM386 module, capacitor C3 on the LM386 module should be bypassed (which is needed so that the LM386 generates a signal between 0..5V and not the default range -5V..+5V).
The LM386 output pin should be connected to an analog pin


#Credits to other projects:
https://wiki.ardumower.de/index.php?title=Perimeter_wire

