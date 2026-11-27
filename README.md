# BeatsAndBars

2HP Eurorack module that shows where you are in a phrase, so you always know when the next transition is coming.

Patch a clock into CLOCK and the module counts four beats per bar across eight bars, using twelve LEDs: four for beats, eight for bars. Exactly one beat LED and one bar LED are lit at all times. A trigger into START resets the count to beat 1, bar 1. If the clock stops for three seconds, the counter resets by itself.

The firmware ignores the clock pulse that arrives with a reset, so the count never runs a beat ahead. It also tolerates a reset arriving slightly after its clock pulse. Tested with Pamela's Pro Workout and Modulove's A-Ryth-Matik.

Built around an ATmega328P and programmed through the Arduino IDE. This repository contains the Gerbers, schematics and firmware.
