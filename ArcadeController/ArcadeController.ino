/*
 *  iElectribe Controller setup for custom Teensy midi controller.
 *  
 *  Author: Juan Segovia
 *  Contact: www.juanlittledevil.com (juanlittledebil at gmail.com)
 *  Creative Commons: NC (Non-Commercial)
 *  https://wiki.creativecommons.org/wiki/4.0/NonCommercial
 *  
 *  Description: First off about the teensy,
 *  
 *  I'm using a Teensy 2.0++ and it is configured as with the following pinout:
 *  
 *  // PUSH BUTTONS
 *  pins 0 to 15 are connected to momentary switches which are arranged in a matrix of 4 x 4 where
 *  pin 0 is on the lower left and 15 is on the upper right.
 *  
 *  // JOYSTICK
 *  pins 16 - 19 connect to an arcade joystick where 16=Right, 17=Down, 18=Left, 19=Up
 *  
 *  // LED
 *  pins 20 - 35 are connected to the matrix as well and match the push buttons so that 20 is on the
 *  lower left and 35 is on the upper right.
 *  
 *  // KNOBS
 *  38 - 45 are the analogue pins, these connect to 8 potentiometers.
 *  
 *  NOTE: Please read the teensy documentation and take special not to to include pullup resistors
 *  where needed. Depending on which version of the teensy you intend to use you may have to use
 *  resistors for a couple of pins connected to switches as well as those which connect to the LEDs.
 *  
 *  the SmoothAnalogInput.h library was installed separately.
 *  https://github.com/rl337/Arduino/tree/master/libraries/SmoothAnalogInput
 *  
 */

#include <Bounce.h>
#include <SmoothAnalogInput.h>


// =======================================================================================
// Variables
// =======================================================================================

bool debug = true;

// We have a 4 x 4 matrix of buttons.
int midi_channel = 10;
const int matrix_size = 16;
const int bounce_delay = 5; // 10ms
const byte max_knobs = 8;
const int max_stick = 4;
int octave = 3;  // default octave start @ C3

int part_selection = 0;


// used to store the pin values.
int push_button_pin[matrix_size] = {
  0, 1, 2, 3,
  4, 5, 6, 7,
  8, 9, 10, 11,
  12, 13, 14, 15
};

// Array of button objects with debounce.
Bounce push_button[] = {
  Bounce(0, bounce_delay), Bounce(1, bounce_delay), Bounce(2, bounce_delay), Bounce(3, bounce_delay),
  Bounce(4, bounce_delay), Bounce(5, bounce_delay), Bounce(6, bounce_delay), Bounce(7, bounce_delay),
  Bounce(8, bounce_delay), Bounce(9, bounce_delay), Bounce(10, bounce_delay), Bounce(11, bounce_delay),
  Bounce(12, bounce_delay), Bounce(13, bounce_delay), Bounce(14, bounce_delay), Bounce(15, bounce_delay)
};

// LED pins
int led[matrix_size] = {
  20, 21, 22, 23,
  24, 25, 26, 27,
  28, 29, 30, 31,
  32, 33, 34, 35
};

// holds the current state of the led matrix.
boolean is_lit[matrix_size] = {
  LOW, LOW, LOW, LOW,
  LOW, LOW, LOW, LOW,
  LOW, LOW, LOW, LOW,
  LOW, LOW, LOW, LOW
};

// holds the current state of the led matrix.
boolean led_fx[4][4] = {
  {20, 21, 22, 23},
  {24, 25, 26, 27},
  {28, 29, 30, 31},
  {32, 33, 34, 35}
};


// joystick
int stick_direction = 0;
int stick_pins[max_stick] = {16, 17, 18, 19};
Bounce stick[] = {
  Bounce(16, bounce_delay), Bounce(17, bounce_delay), Bounce(18, bounce_delay), Bounce(19, bounce_delay)
};


// knobs  
int knob_pins[max_knobs] = {38, 39, 40, 41, 42, 43, 44, 45};    // teensy pin values
int knob_state[max_knobs] = {0, 0, 0, 0, 0, 0, 0, 0};           // initialize the knob states
int knob_prev_state[max_knobs] = {0, 0, 0, 0, 0, 0, 0, 0};      // used to compare updates with previous state. Did it change?

SmoothAnalogInput knobs[max_knobs];                             // create array of SmoothAnalogInput objects.

//           [part][cc_number]
int part_midi_map[16][8];

//         [midi_ch][part][cc_number]
int part_midi_state[16][16][8];

//      [octave][note]
int midi_note[11][12];

//     [scale][note_key]
int scales[16][8] = {
 //1,2,3,4,5,6, 7,8
  {0,2,4,5,7,9,11,0}, // Major                w-w-h-w-w-w-h
  {0,2,3,5,7,8,10,0}, // Natural Minor        w-h-w-w-h-w-w
  {0,2,3,5,7,8,11,0}, // Harmonic Minor       w-h-w-w-h-3-h
  {0,2,3,5,7,9,11,0}, // Jazz/Melodic Minor   w-h-w-w-w-w-h
  {0,2,3,5,7,9,10,0}, // Dorian               w-h-w-w-w-h-w
  {0,1,3,5,7,8,10,0}, // Phrygian             h-w-w-w-h-w-w
  {0,2,4,5,7,8,10,0}, // Spanish              h-3-h-w-h-w-w
  {0,2,4,6,7,9,11,0}, // Lydian               w-w-w-h-w-w-h
  {0,2,3,5,6,9,11,0}, // Diminished           w-h-w-h-w-h-w
  {0,1,4,5,6,8,11,0}, // Persian              h-3-h-h-w-3-h
  {0,2,4,5,6,8,10,0}, // Arabian              w-w-h-h-w-w-w
  {0,3,3,5,6,7,10,0}, // Blues                3-w-h-h-3-w
  {0,2,5,5,7,9, 9,0}, // Scottish             w-3-2-2-3
  {0,2,3,3,7,8, 8,0}, // Hirojoshi            w-h-3-h-3
  {0,2,3,7,7,9, 9,0}, // Major Pentatonic     w-w-3-w-3
  {0,3,3,5,7,7,10,0}  // Minor Pentatonic     3-w-w-3-w
};

int play_note[16];

// Timer matrix, match the index to pair them up.
int tick[] = {0};
const int tock[] = {200};

// =======================================================================================
// Methods
// =======================================================================================
// init as it states gets ran from setup()
void init_hardware() {
  for ( int i=0; i < matrix_size; i++ ) {
    pinMode(push_button_pin[i], INPUT_PULLUP);
    pinMode(led[i], OUTPUT);
  }

  for ( int i=0; i < max_knobs; i++ ) {
    knobs[i].attach(knob_pins[i]);
  }

  for ( int i=0; i < max_stick; i++) {
    pinMode(stick_pins[i], INPUT_PULLUP);
  }
}

// Setup the midi matrix.
void init_midi_map() {
  int cc = 0;
  for ( int butt = 0; butt < 16; butt++) {
    for ( int pot = 0; pot < 8; pot++ ) {
      part_midi_map[butt][pot] = cc;
      cc++;
    }
  }

  for ( int midi_ch = 0; midi_ch < 16; midi_ch++ ) {
    for ( int butt = 0; butt < 16; butt++) {
      for ( int pot = 0; pot < 8; pot++ ) {
        part_midi_state[midi_ch][butt][pot] = 0;
      }
    }
  }

  int note_val = 0;
  for ( int oct = 0; oct < 11; oct++ ) {
    for ( int note = 0; note < 12; note++ ) {
      midi_note[oct][note] = note_val;
      note_val++;
    }
  }
}

void update_play_note() {
  // initialize the scale with major.
  for (int key = 0; key < 8; key++){
    if ( key != 7 ){
      play_note[key] = midi_note[octave][scales[0][key]];
    } else {
      play_note[key] = midi_note[octave + 1][scales[0][key]];
    }
  }
  for (int key = 8; key < 16; key++){
    if ( key != 15 ){
      play_note[key] = midi_note[octave + 1][scales[0][key]];
    } else {
      play_note[key] = midi_note[octave + 2][scales[0][key]];
    }
  }
}


// Flashy splash sequence at boot time.
void boot_sequence() {
  for (int i=0; i < matrix_size; i++ ) {
    digitalWrite(led[i], HIGH);
    delay(25);
    digitalWrite(led[i], LOW);
    delay(10);
  }
}


// Button states - this method gets ran from loop()
// This is is how we get the state of the buttons and do stuff.
void update_button_states() {
  for ( int butt=0; butt < matrix_size; butt++ ) {
    if (push_button[butt].update()) {
      // LOW is means the button has been pressed.
      if (push_button[butt].read() == LOW) {
        
        //   -- Center --

        if ( stick_direction == 0 ) {
          // Send note on --
          play_notes(butt, HIGH);
        } 
        
        //   -- Right --
        // Update the last step value cc 6 value.
        if ( stick_direction == 1 ) {
          // usbMIDI.sendControlChange(5, butt + 1, midi_channel);
          // if (debug == true) {
            // Serial.print("update_button_states(cc, value)");
            // print_debug(5, butt + 1);
          // }
        }
        
        //   --  UP --
        if ( stick_direction == 2 ) {
          select_cc_bank(butt);
        }

        //   --  Left --
        // Update the loop length with the 4 bottom buttons.
        if ( stick_direction == 3 ) {
          transpose(butt);
        }

        //   -- Down --
        // Mute the parts
        if ( stick_direction == 4 ) {
          select_midi_channel(butt);
        }

        
        // Light when the button is pressed
        is_lit[butt] = HIGH;
        
      // HIGH (else) means the button has been released.
      } else {
        // Send note off --
        play_notes(butt, LOW);
        is_lit[butt] = LOW;

        // Visual Effects.
        if (stick_direction !=0) {
          shoot_ray(butt);
        }
      }
    }
  }
}


void select_midi_channel(int butt) {
  midi_channel = butt + 1;
  if (debug == true) {
    Serial.print("select_midi_channel(button, midi_channel)");
    print_debug(butt, midi_channel);
  }
}


void display_midi_channel(int index) { // timer returns boolean in a cycle.
  if ( tick[index] <= tock[index] / 2 ) {
    if ( stick_direction == 4 ) {
      digitalWrite(led[midi_channel - 1], HIGH);
    }
  }
  if ( tick[index] >= tock[index] / 2 ) {
    if ( stick_direction == 4 ) {
      digitalWrite(led[midi_channel - 1], LOW);
    }
  }
  if ( tick[index] == tock[index] ) {
    tick[index] = 0;
  }
  tick[index]++;
//  if (debug == true) {
//    Serial.print("display_midi_channel(tick[index], midi_channel)");
//    print_debug(tick[index], midi_channel);
//  }
}


void transpose(int butt) {
  octave = butt;
  update_play_note();
  if (debug == true) {
    Serial.print("transpose(button, octave)");
    print_debug(butt, octave);
  }
}


void select_cc_bank(int butt) {
  part_selection = butt;
  if (debug == true) {
    Serial.print("select_cc_bank(button, part_selection)");
    print_debug(butt, part_selection);
  }
}


// This is what happens when we push the buttons.
void play_notes(int butt, boolean on) {
  // == PART NOTES ==
  if (on == HIGH) {
    usbMIDI.sendNoteOn(play_note[butt], 99, midi_channel);
    if (debug == true) {
      Serial.print("play_notes(note, midi_channel)");
      print_debug(play_note[butt], midi_channel);
    }
  } else if (on == LOW) {
    usbMIDI.sendNoteOff(play_note[butt], 0, midi_channel);
  }
}


// Update Stick - Ran from loop().
void update_stick_states() {
  for ( int i=0; i < max_stick; i++ ) {
    if (stick[i].update()) {
      if (stick[i].read() == LOW) {
        detect_direction(i, HIGH);
      } else {
        detect_direction(i, LOW);
      }
    }
  }
}


// Which direction is the Joystick pressed?
void detect_direction(int i, boolean on) {
  if (on == HIGH) {

    if (i == 0) {
      // Right
      stick_direction = 1;
    } else if (i == 1) {
      // Up
      stick_direction = 2;
    } else if (i == 2) {
      // Left
      stick_direction = 3;
    } else if (i == 3) {
      // down
      stick_direction = 4;
    }
  } else {
    // Center
    stick_direction = 0;
  }
  if (debug == true) {
    Serial.print("detect_direction(pin, direction)");
    print_debug(stick_pins[i], stick_direction);
  }
}


// Turn on leds, or off depending on our state matrix.
void update_leds() {
  for (int i=0; i < matrix_size; i++ ) {
    if (i == part_selection) {
      digitalWrite(led[i], HIGH);
      delay(1);
      digitalWrite(led[i], LOW);
      delay(1);
    }
    digitalWrite(led[i], is_lit[i]);
  }
}


// Visual Effect that shoots a ray in the direction of the stick when a button is pressed.
// input i: the button number where 0 is the bottom left and 15 is the top right of the matrix.
void shoot_ray(int i) {
  int ray_delay = 15;
  if (stick_direction == 1) {
    //      RIGHT
    if (debug == true) {
      Serial.println("right");
    }
    for (int x=0; x < 4; x++) {
      for (int y=0; y < 4; y++) {
        if (led_fx[x][y] == led[i]) {
          for (int a=y; a < 4; a++) {
            digitalWrite(led_fx[x][a], HIGH);
            delay(ray_delay);
            digitalWrite(led_fx[x][a], LOW);
          }
        }
      }
    }

  } else if (stick_direction ==2) {
    //      UP
    if (debug == true) {
      Serial.println("up");
    }
    for (int x=i; x < 16; x = x + 4) {
      digitalWrite(led[x], HIGH);
      delay(ray_delay);
      digitalWrite(led[x], LOW);
    }

  } else if (stick_direction ==3) {
    //      LEFT
    if (debug == true) {
      Serial.println("left");
    }
    for (int x=0; x < 4; x++) {
      for (int y=0; y < 4; y++) {
        if (led_fx[x][y] == led[i]) {
          for (int a=y; a >= 0; a--) {
            digitalWrite(led_fx[x][a], HIGH);
            delay(ray_delay);
            digitalWrite(led_fx[x][a], LOW);
          }
        }
      }
    }

  } else if (stick_direction ==4) {
    //      DOWN
    if (debug == true) {
      Serial.println("down");
    }
    for (int x=i; x >= 0; x = x - 4) {
      digitalWrite(led[x], HIGH);
      delay(ray_delay);
      digitalWrite(led[x], LOW);
    }

  }
  if (debug == true) {
    Serial.print("shoot_ray(stick_direction, button)");
    print_debug(stick_direction, i);
  }
}


// Knob states...
void update_knob_states() {
  for (int i=0; i < max_knobs; i++ ) {
    knob_state[i] = map(knobs[i].read(), 0, 1024, 0, 128);
    if ( knob_state[i] != knob_prev_state[i] ) {
      part_midi_state[midi_channel - 1][part_selection][i] = knob_state[i];

      usbMIDI.sendControlChange(part_midi_map[part_selection][i], part_midi_state[midi_channel - 1][part_selection][i], midi_channel);

      if (debug == true) {
        Serial.print("update_knob_states(cc, value)");
        print_debug(part_midi_map[part_selection][i], part_midi_state[midi_channel - 1][part_selection][i]);
      }
    }
    knob_prev_state[i] = knob_state[i];
  }
}


// Tool for debugging output into serial console.
void print_debug(int arg1, int arg2) {
  Serial.print(" [");
  Serial.print(arg1);
  Serial.print("]: ");
  Serial.print(arg2);
  Serial.println("");
}


// =======================================================================================
// Setup
// =======================================================================================
void setup() {
  Serial.begin(9600);
  init_hardware();
  init_midi_map();
  update_play_note();
  boot_sequence();
}


// =======================================================================================
// The loop()
// =======================================================================================
void loop() {
  update_button_states();
  update_stick_states();
  update_knob_states();
  update_leds();
  display_midi_channel(0);

  while (usbMIDI.read()) {
    // ignore incoming messages
  }
}



