#  Arcade Tank Stick V2.0

### Author: Juan Segovia
### Contact: www.juanlittledevil.com (juanlittledevil at gmail.com)
### Creative Commons: NC (Non-Commercial)
### https://wiki.creativecommons.org/wiki/4.0/NonCommercial

## Description:

First off about the teensy,

I'm using a Teensy 2.0++ and it is configured as with the following pinout:

// PUSH BUTTONS
pins 0 to 15 are connected to momentary switches which are arranged in a matrix of 4 x 4 where
pin 0 is on the lower left and 15 is on the upper right.

// JOYSTICK
pins 16 - 19 connect to an arcade joystick where 16=Right, 17=Down, 18=Left, 19=Up

// LED
pins 20 - 35 are connected to the matrix as well and match the push buttons so that 20 is on the
lower left and 35 is on the upper right.

// KNOBS
38 - 45 are the analogue pins, these connect to 8 potentiometers.

NOTE: Please read the teensy documentation and take special not to to include pullup resistors
where needed. Depending on which version of the teensy you intend to use you may have to use
resistors for a couple of pins connected to switches as well as those which connect to the LEDs.

the SmoothAnalogInput.h library was installed separately.
https://github.com/rl337/Arduino/tree/master/libraries/SmoothAnalogInput

## MIDI implementation and manual.

The joystick is used to select different modes.

### UP - Channel Slect.

To select the MIDI Channel hold the joystick in the up position. Part selection is as follows.

	`    ^          (13)    (14)    (15)    (16) `
	`    |          ( 9)    (10)    (11)    (12) `
	`               ( 5)    ( 6)    ( 7)    ( 8) `
	`               ( 1)    ( 2)    ( 3)    ( 4) `


### DOWN

There are 16 bank of 8 knobs. Each knob will send a MIDI cc message. The knobs are arranged as shown here.

	'     (1)  (2)  (3)  (4)      '
	'     (5)  (6)  (7)  (8)      '

You have access to all 128 control message parameters. Pressing down button (1) while holding the joystick
 in the down position will set the knobs with the first 8 messages, starting from cc 0 (bank select) to cc 7 (volume). 
 Pressing the next button, button (2), will set the knobs from cc 8 to cc 15 and so on.
 For info on MIDI values see. http://nickfever.com/music/midi-cc-list

The following shows the bank order.

	`               (13)    (14)    (15)    (16) `
	`               ( 9)    (10)    (11)    (12) `
	`    |          ( 5)    ( 6)    ( 7)    ( 8) `
	`    v          ( 1)    ( 2)    ( 3)    ( 4) `


### LEFT

	`               (13)    (14)    (15)    (16) `
	`  <--          ( 9)    (10)    (11)    (12) `
	`               ( 5)    ( 6)    ( 7)    ( 8) `
	`               ( 1)    ( 2)    ( 3)    ( 4) `


### RIGHT

	`               (13)    (14)    (15)    (16) `
	`    -->        ( 9)    (10)    (11)    (12) `
	`               ( 5)    ( 6)    ( 7)    ( 8) `
	`               ( 1)    ( 2)    ( 3)    ( 4) `



### CENTER

This is the default mode.

## Final words... 

I'm not a C++ programmer and was not able to get eclipse arduino to work so I was
forced to use the arduino IDE which is kinda lame IMHO. There are some things in here which would have
benefited from making them into objects, instead I had to do some mad multi-dimentional arrays.
The result was satisfactory tho. Have fun with this, I hope you enjoy it as much as I have been.
