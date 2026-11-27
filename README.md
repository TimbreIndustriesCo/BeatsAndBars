# Beats & Bars

2HP Eurorack module that shows where you are in a phrase, so you always know when the next transition is coming.

Patch a clock into CLOCK and the module counts four beats per bar across eight bars, using twelve LEDs: four for beats, eight for bars. Exactly one beat LED and one bar LED are lit at all times. A trigger into START resets the count to beat 1, bar 1. If the clock stops for three seconds, the counter resets by itself.

The firmware ignores the clock pulse that arrives with a reset, so the count never runs a beat ahead. It also tolerates a reset arriving slightly after its clock pulse. Tested with Pamela's Pro Workout and Modulove's A-Ryth-Matik.

Built around an ATmega328P and programmed through the Arduino IDE. This repository contains the Gerbers, schematics and firmware.

If you want to order this module and assemble it yourself, order the three Gerber files (BeatsAndBarsFrontpanel.zip, BeatsAndBarsSMD.zip and BeatsAndBarsTH.zip) from any PCB manufacturer and all the parts from the BOM. Do note that this is a build only for experienced solderers, with several SMD components.

You can, however, order all SMD parts assembled on JLCPCB. Upload the BeatsAndBarsSMD.zip as gerber, select SMD assembly and upload the JLCPCBBOMSMD.xlsx and JLCPCBPickAndPLaceSMD.xlsx when prompted. This way, only the 90 degree headers, LEDs and jacks have to be soldered on, together with a 2.5mm standoff. Good luck building it!
