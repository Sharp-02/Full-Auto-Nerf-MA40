# Full-Auto-Nerf-MA40

# Introduction
Recently, I took Nerf's Halo MA40 Blaster and modded it to have stronger firing(rev) motors
and select fire capability (full-auto, burst fire, semi-auto). When stock, this nerf blaster
shoots darts at a speed close to 70 feet per second (FPS). Modified, it shoots at an average
speed of 118 FPS in full-auto, and 130 FPS in semi-auto.

# Design Goals:
- External Power Switch
- Motor Upgrade
- Dynamic Braking
- Motor Speed Control
- Select Fire
 - Semi-Automatic Firing
 - Fully Automatic Firing
 - 3 Burst Firing


# Power Switch
This may seem like a basic function, but stock, the MA40 does not have an external power switch.
I added one where the blaster had a design that looked similar to a switch. Because the rocker
switches I had cannot handle the motor's drawn current, and because I wanted to mount the rocker
opposite to the main half of the Nerf blaster's shell, I use the switch to instead bias a MOSFET.
Current to all components can only flow through this MOSFET.


# Motor Upgrade
Replacing the motors is not something that can be done in isolation. To replace the motors,
you have to replace the motor cage as well. The batteries must be replaced too. I had to 
hollow out the shell so that my new battery could fit. It took a set of AA batteries before,
but runs off a 2S LiPo (7.4 Volt Lithium Polymer) battery now.

You also need to replace any electronic components that are in series with the motors 
to components that will be able to handle the higher current your motors will draw.
What is commonly done is a replacement of the rev trigger switch, which normally is
in series with the motors. I opted to keep the stock switch and instead have it control
an IRL7833 N-Channel MOSFET. The IRL7833 then controls the voltage supplied to the motors.

Finally, add a flywheel/snubber diode in parallel with the motors. The diode should be reverse
biased when the motors are on (arrow points to the positive end of the battery). This diode
takes the brunt of the negative kick the motors will generate when they are turned off.


# Solenoid Integration
Out of the box, the Nerf blaster uses a mechanical linkage between the trigger and a pusher
that pushes the loaded dart into the motor cage. When modifying the firing mechanism, I 
replaced the trigger linkage with a generic 12V solenoid with a stroke length of 35mm. 
I got mine off Amazon, but you can also get them off sites like AliExpress. I 3D printed a
custom part to connect the solenoid to the pusher bar. The solenoid is also controlled by the
same MOSFET used with the motors.

Originally, I hoped to use a voltage booster to supply 16 volts to the solenoid, but there
were no affordable options within the power drawn by the solenoid that could fit inside the
Nerf blaster. After multiple failed attempts with different voltage multipliers, I ended up
using a separate 4S LiPo Battery (14.8 V) to supply power to the solenoid.


# Microcontroller Operation
To achieve all the design goals, it's easiest to use a microcontroller board like the Arduino
Nano to control all aspects of the Nerf blaster. The Arduino reads multiple sensors, then
controls the MOSFETs that, in turn, control the solenoid and the motors. Due to both the 
motors' and solenoid's induced voltages, a voltage regulator needs to be used to protect 
the Arduino. I used the LM7805 Voltage Regulator as they are abundant and provide the 5V 
needed for the Arduino.

The Arduino reads signals from the rev trigger switch and the firing trigger switch. While I
was able to use the pre-installed rev trigger switch, I had to fasten a limit switch that is
triggered by the firing trigger. When the rev trigger is pulled, the motors are turned on.
To control their speed, a PWM capable output pin on the Arduino is used to bias the
motor MOSFETs. The solenoid is coded to only turn on while the motors are turned on. This
prevents dart jams in the case that a dart is pushed between the motors while they are not
running. 

To control motor speed, the Arduino reads the analog voltage at a 10k, 300 degree potentiometer
mounted on the shell.

The solenoid is turned on and off at a user-defined frequency, which is 10Hz in my case. The
solenoid is turned on for a set time, then turned off for the rest of the cycle. The "on time"
is determined by the voltage applied to the solenoid. Ideally, the higher the voltage supplied,
the stronger the stroke of the armature. At low voltages, the solenoid does not have enough 
speed or power to push a dart at any usable rate. The frequency/"off time" is determined by
the strength of the return spring. The stronger the return spring is, the sooner the solenoid
will be ready to fire. This is a balancing act between supply voltage, spring strength, and
the on/off times.

To control firing modes, the Arduino reads the analog voltage of a "selector switch" circuit.
The switch used is a 3 position (SP3T) rotary switch, with each position connected to resistors
of different values. This acts as a 'discrete potentiometer' of sorts, where the analog voltage
can be read, using only one wire to read a signal.

