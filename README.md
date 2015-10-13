#  Arcade MIDI Stick V2.0

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

A blinking light will indicate the currently selected MIDI channel as long as the stick is in the up position.

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

A solid lit led will indicate the currently selected part.


### LEFT

Press and hold left on the stick while pressing a button to select which octave the buttons will play. The default
is C3 where C3 (note 36) is mapped to the bottom left button and so on. You can easily navigate through the entire
key range this way. By default pressing keys will play a chromatic scale, however this can be changed see below. A
blinking light will indicate the current octave.

	`               (13)    (14)    (15)    (16) `
	`  <--          ( 9)    (10)    (11)    (12) `
	`               ( 5)    ( 6)    ( 7)    ( 8) `
	`               ( 1)    ( 2)    ( 3)    ( 4) `


### RIGHT

	`               (13)    (14)    (15)    (16) `
	`    -->        ( 9)    (10)    (11)    (12) `
	`               ( 5)    ( 6)    ( 7)    ( 8) `
	`               ( 1)    ( 2)    ( 3)    ( 4) `

The right direction of the stick enables a multi-mode selection as follows.

## Key selection

While holding the stick to the right position press button (1) to cycle through the 12 keys. By default the scale
will play in the key of C, but pressing the button will transpose the scale by half a step. An slow blinking LED
will light up and indicate which key is currently selected. The lit LED maps as follows:

	(1)     -     C
	(2)     -     C#
	(3)     -     D
	(4)     -     D#
	(5)     -     E
	(6)     -     F
	(7)     -     F#
	(8)     -     G
	(9)     -     G#
	(10)    -     A
	(11)    -     A#
	(12)    -     B

## scale selection

While holding the stick to the right position press button (2) to cycle through 17 scales. By default the scale played
is a chromatic scale with all 12 steps. However, enabling scales will change it so that only keys in that particular
scale are played. A fast led will light up indicating the currently selected scale but only for scales 1 to 16.
scale 17 (chromatic) will not show a fast blinking light, you will still see the slow blinking light however.
Here is a breakdown of the different scale modes:

	(1)    Major                w-w-h-w-w-w-h
	(2)    Natural Minor        w-h-w-w-h-w-w
	(3)    Harmonic Minor       w-h-w-w-h-3-h
	(4)    Jazz/Melodic Minor   w-h-w-w-w-w-h
	(5)    Dorian               w-h-w-w-w-h-w
	(6)    Phrygian             h-w-w-w-h-w-w
	(7)    Spanish              h-3-h-w-h-w-w
	(8)    Lydian               w-w-w-h-w-w-h
	(9)    Diminished           w-h-w-h-w-h-w
	(10)   Persian              h-3-h-h-w-3-h
	(11)   Arabian              w-w-h-h-w-w-w
	(12)   Blues                3-w-h-h-3-w
	(13)   Scottish             w-3-2-2-3
	(14)   Hirojoshi            w-h-3-h-3
	(15)   Major Pentatonic     w-w-3-w-3
	(16)   Minor Pentatonic     3-w-w-3-w
	(17)   (defualt) chromatic.


### CENTER

This is the default mode. In this mode the buttons imply play midi notes. While in the center position

