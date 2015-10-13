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

bool debug = false;
bool latch_mode = true;

// We have a 4 x 4 matrix of buttons.
byte midi_channel = 10;
const byte matrix_size = 16;
const byte bounce_delay = 5; // 10ms
const byte max_knobs = 8;
const byte max_stick = 4;
const byte right = 1;
const byte up = 2;
const byte left = 3;
const byte down = 4;
byte octave = 3;  // default octave start @ C3
byte key = 0;     // 0 is C each value is a half step.
byte scale = 15;        // this select the index of scales[x][8]. default to chromatic.

byte part_selection = 0;


// used to store the pin values.
byte push_button_pin[matrix_size] = {
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
byte led[matrix_size] = {
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
byte stick_direction = 0;
byte stick_pins[max_stick] = {16, 17, 18, 19};
Bounce stick[] = {
  Bounce(16, bounce_delay), Bounce(17, bounce_delay), Bounce(18, bounce_delay), Bounce(19, bounce_delay)
};


// knobs  
byte knob_pins[max_knobs] = {38, 39, 40, 41, 42, 43, 44, 45};    // teensy pin values
byte knob_state[max_knobs] = {0, 0, 0, 0, 0, 0, 0, 0};           // initialize the knob states


//            [midi_ch][part][cc_number]
byte knob_prev_state[16][16][max_knobs];                 // used to compare updates with previous state. Did it change?

SmoothAnalogInput knobs[max_knobs];                             // create array of SmoothAnalogInput objects.

//           [part][cc_number]
byte part_midi_map[16][max_knobs];

//         [midi_ch][part][cc_number]
byte part_midi_state[16][16][max_knobs];

//      [octave][note]
byte midi_note[11][12];

//     [scale][note_key]
byte scales[17][12] = { // workaround for reset.
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
  {0,3,3,5,7,7,10,0}, // Minor Pentatonic     3-w-w-3-w
  {0,1,2,3,4,5, 6,7,8,9,10,11}  // defualt to chromatic.
};

byte update_flag[16][16][8];

byte play_note[16];

// Timer matrix, match the index to pair them up.
byte tick[] = {0,0,0,0,0};
const byte tock[] = {200,100,200,25,10};

// =======================================================================================
// Methods
// =======================================================================================
void init_hardware() {
// init as it states gets ran from setup()
  for ( byte i=0; i < matrix_size; i++ ) {
    pinMode(push_button_pin[i], INPUT_PULLUP);
    pinMode(led[i], OUTPUT);
  }

  for ( byte i=0; i < max_knobs; i++ ) {
    knobs[i].attach(knob_pins[i]);
  }

  for ( byte i=0; i < max_stick; i++) {
    pinMode(stick_pins[i], INPUT_PULLUP);
  }
}

void init_midi_map() {
// Setup the midi matrix.
  byte cc = 0;
  for ( byte butt = 0; butt < 16; butt++) {
    for ( byte pot = 0; pot < 8; pot++ ) {
      part_midi_map[butt][pot] = cc;
      cc++;
    }
  }

  for ( byte midi_ch = 0; midi_ch < 16; midi_ch++ ) {
    for ( byte butt = 0; butt < 16; butt++) {
      for ( byte pot = 0; pot < 8; pot++ ) {
        part_midi_state[midi_ch][butt][pot] = 0;
      }
    }
  }

  for ( byte midi_ch = 0; midi_ch < 16; midi_ch++ ) {
    for ( byte butt = 0; butt < 16; butt++) {
      for ( byte pot = 0; pot < 8; pot++ ) {
        knob_prev_state[midi_ch][butt][pot] = 0;
      }
    }
  }

  byte note_val = 0;
  for ( byte oct = 0; oct < 11; oct++ ) {
    for ( byte note = 0; note < 12; note++ ) {
      midi_note[oct][note] = note_val;
      note_val++;
    }
  }
}

void update_play_note() {
  // initialize the scale with major.
  if ( scale < 16 ) {
    for (byte note = 0; note < 8; note++){
      if ( note != 7 ){
        play_note[note] = midi_note[octave][scales[scale][note]] + key;
      } else {
        play_note[note] = midi_note[octave + 1][scales[scale][note]] + key;
      }
    }
    for (byte note = 8; note < 16; note++){
      if ( note != 15 ){
        play_note[note] = midi_note[octave + 1][scales[scale][note - 8]] + key;
      } else {
        play_note[note] = midi_note[octave + 2][scales[scale][note - 8]] + key;
      }
    }
  } else if (scale == 16) {
    for (byte note = 0; note < 12; note++) {
      play_note[note] = midi_note[octave][scales[16][note]] + key;
    }
    for (byte note = 12; note < 16; note++) {
      play_note[note] = midi_note[octave + 1][scales[16][note - 12]] + key;
    }
  }
}

void boot_sequence() {
// Flashy splash sequence at boot time.
  for (byte i=0; i < matrix_size; i++ ) {
    digitalWrite(led[i], HIGH);
    delay(25);
    digitalWrite(led[i], LOW);
    delay(10);
  }
}

void update_button_states() {
// Button states - this method gets ran from loop()
// This is is how we get the state of the buttons and do stuff.
  for ( byte butt=0; butt < matrix_size; butt++ ) {
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

        if ( stick_direction == right ) {
          if ( butt == 0 ) {  // Select Key
            // Essentially a transpose value for the scales.
            select_key(butt);
          }
          if ( butt == 1 ) { // Select scale
            select_scale(butt);
          }
          if ( butt == 2 ) { // Select scale
            select_latch_mode(butt);
          }
        }
        
        //   --  UP --
        if ( stick_direction == up ) {
          select_midi_channel(butt);
        }

        //   --  Left --
        // Update the loop length with the 4 bottom buttons.
        if ( stick_direction == left ) {
          transpose(butt);
        }

        //   -- Down --
        // Mute the parts
        if ( stick_direction == down ) {
          select_cc_bank(butt);
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

void select_midi_channel(byte butt) {
  midi_channel = butt + 1;
  if (debug == true) {
    Serial.print("select_midi_channel(button, midi_channel)");
    print_debug(butt, midi_channel);
  }
}

void select_scale(byte butt) {
  scale++;
  if ( scale == 16 ) {
    scale = 0;
  }
  if (debug == true) {
    Serial.print("select_key(butt, scale)");
    print_debug(butt, scale);
  }
}

void select_latch_mode(byte butt) {
  if (latch_mode == true) {
    latch_mode = false;
  } else {
    latch_mode = true;
  }
  if (debug == true) {
    Serial.print("select_latch_mode(butt, latch_mode)");
    print_debug(butt, latch_mode);
  }
}

void select_key(byte butt) {
  key++;
  if ( key == 12 ) {
    key = 0;
  }
  if (debug == true) {
    Serial.print("select_key(butt, key)");
    print_debug(butt, key);
  }
}

void display_octave(byte index) {
  if ( tick[index] <= tock[index] / 2 ) {
    if ( stick_direction == left ) {
      digitalWrite(led[octave], HIGH);
    }
  }
  if ( tick[index] >= tock[index] / 2 ) {
    if ( stick_direction == left ) {
      digitalWrite(led[octave], LOW);
    }
  }
  if ( tick[index] == tock[index] ) {
    tick[index] = 0;
  }
  tick[index]++;
}

void display_latch_mode(byte index) {
  if ( tick[index] <= tock[index] / 2 ) {
    if ( stick_direction == right && latch_mode == true ) {
      digitalWrite(led[2], HIGH);
    }
  }
  if ( tick[index] >= tock[index] / 2 ) {
    if ( stick_direction == right && latch_mode == true ) {
      digitalWrite(led[2], LOW);
    }
  }
  if ( tick[index] == tock[index] ) {
    tick[index] = 0;
  }
  tick[index]++;
}

void display_scale(byte index) {
  if ( tick[index] <= tock[index] / 2 ) {
    if ( stick_direction == right && scale < 16) {
      digitalWrite(led[scale], HIGH);
    }
  }
  if ( tick[index] >= tock[index] / 2 ) {
    if ( stick_direction == right && scale < 16) {
      digitalWrite(led[scale], LOW);
    }
  }
  if ( tick[index] == tock[index] ) {
    tick[index] = 0;
  }
  tick[index]++;
}

void display_key(byte index) {
  if ( tick[index] <= tock[index] / 2 ) {
    if ( stick_direction == right ) {
      digitalWrite(led[key], HIGH);
    }
  }
  if ( tick[index] >= tock[index] / 2 ) {
    if ( stick_direction == right ) {
      digitalWrite(led[key], LOW);
    }
  }
  if ( tick[index] == tock[index] ) {
    tick[index] = 0;
  }
  tick[index]++;
}

void display_midi_channel(byte index) {
  if ( tick[index] <= tock[index] / 2 ) {
    if ( stick_direction == up ) {
      digitalWrite(led[midi_channel - 1], HIGH);
    }
  }
  if ( tick[index] >= tock[index] / 2 ) {
    if ( stick_direction == up ) {
      digitalWrite(led[midi_channel - 1], LOW);
    }
  }
  if ( tick[index] == tock[index] ) {
    tick[index] = 0;
  }
  tick[index]++;
}

void transpose(byte butt) {
  octave = butt;
  update_play_note();
  if (debug == true) {
    Serial.print("transpose(button, octave)");
    print_debug(butt, octave);
  }
}

void select_cc_bank(byte butt) {
  part_selection = butt;
  if ( latch_mode == true ) {
    set_update_flag(false);
  } else {
    set_update_flag(true);
  }
  if (debug == true) {
    Serial.print("select_cc_bank(button, part_selection)");
    print_debug(butt, part_selection);
  }
}

void set_update_flag(bool state) {
  for (byte m_channel=0; m_channel < 16; m_channel++) {
    for (byte part=0; part < 16; part++) {
      for (byte knob=0; knob < 8; knob++) {
        update_flag[m_channel][part][knob] = state;
      }
    }
  }
}

void play_notes(byte butt, boolean on) {
  // This is what happens when we push the buttons.
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

void update_stick_states() {
  // Update Stick - Ran from loop().
  for ( byte i=0; i < max_stick; i++ ) {
    if (stick[i].update()) {
      if (stick[i].read() == LOW) {
        detect_direction(i, HIGH);
      } else {
        detect_direction(i, LOW);
      }
    }
  }
}

void detect_direction(byte i, boolean on) {
// Which direction is the Joystick pressed?
  if (on == HIGH) {

    if (i == 0) {
      // Right
      stick_direction = right;
    } else if (i == 1) {
      // Up
      stick_direction = up;
    } else if (i == 2) {
      // Left
      stick_direction = left;
    } else if (i == 3) {
      // down
      stick_direction = down;
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

void update_leds() {
// Turn on leds, or off depending on our state matrix.
  for (byte i=0; i < matrix_size; i++ ) {
    if (i == part_selection && stick_direction != right) {
      digitalWrite(led[i], HIGH);
      delay(1);
      digitalWrite(led[i], LOW);
      delay(1);
    }
    digitalWrite(led[i], is_lit[i]);
  }
}

void shoot_ray(byte i) {
// Visual Effect that shoots a ray in the direction of the stick when a button is pressed.
// input i: the button number where 0 is the bottom left and 15 is the top right of the matrix.
  byte ray_delay = 15;
  if (stick_direction == right) {       //      RIGHT
/*    for (byte x=0; x < 4; x++) {    // This one isn't affecting anything, but it looks odd..
      for (byte y=0; y < 4; y++) {
        if (led_fx[x][y] == led[i]) {
          for (byte a=y; a < 4; a++) {
            digitalWrite(led_fx[x][a], HIGH);
            delay(ray_delay);
            digitalWrite(led_fx[x][a], LOW);
          }
        }
      }
    } */

  } else if (stick_direction == up) { //      UP
/*    for (byte x=i; x >= 0; x = x - 4) { // this is causing weird behavior, I think it uses up all little RAM left.
      digitalWrite(led[x], HIGH);         // I'm gonna turn it off, it's cosmetic anyway.
      delay(ray_delay);
      digitalWrite(led[x], LOW);
    }*/

  } else if (stick_direction == left) { //      LEFT
/*    for (byte x=0; x < 4; x++) { // this one is also eating up mem so, better turn it off as well.
      for (byte y=0; y < 4; y++) {
        if (led_fx[x][y] == led[i]) {
          for (byte a=y; a >= 0; a--) {
            digitalWrite(led_fx[x][a], HIGH);
            delay(ray_delay);
            digitalWrite(led_fx[x][a], LOW);
          }
        }
      }
    } */

  } else if (stick_direction == down) { //      DOWN
    for (byte x=i; x < 16; x = x + 4) {
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

void update_knob_states() {
// Knob states...
  for (byte i=0; i < max_knobs; i++ ) {
    knob_state[i] = map(knobs[i].read(), 0, 1024, 0, 128);
    if ( knob_state[i] == knob_prev_state[midi_channel -1][part_selection][i] ) {
      update_flag[midi_channel][part_selection][i] = true;
    }
  }
  for (byte i=0; i < max_knobs; i++ ) {
    if ( update_flag[midi_channel][part_selection][i] == true ){
      knob_state[i] = map(knobs[i].read(), 0, 1024, 0, 128);
      if ( knob_state[i] != knob_prev_state[midi_channel -1][part_selection][i] ) {
        part_midi_state[midi_channel - 1][part_selection][i] = knob_state[i];

        usbMIDI.sendControlChange(part_midi_map[part_selection][i],
                                  part_midi_state[midi_channel - 1][part_selection][i],
                                  midi_channel
                                  );

        if (debug == true) {
          Serial.print("update_knob_states(cc, value)");
          print_debug(part_midi_map[part_selection][i],
                                    part_midi_state[midi_channel - 1][part_selection][i]
                                    );
        }
      }
      knob_prev_state[midi_channel -1][part_selection][i] = knob_state[i];
    }
  }
}

void print_debug(byte arg1, byte arg2) {
// Tool for debugging output into serial console.
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
  display_octave(1);
  display_key(2);
  display_scale(3);
  display_latch_mode(4);

  while (usbMIDI.read()) {
    // ignore incoming messages
  }
}



