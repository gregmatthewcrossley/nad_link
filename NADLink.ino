/*

Controls the NAD C740 Receiver via NADLink 

Based on this project: https://github.com/bitraf/nadlink/

NEC IR Protocal info here:
https://techdocs.altium.com/display/FPGA/NEC+Infrared+Transmission+Protocol
https://www.sbprojects.net/knowledge/ir/nec.php

TO-DO
- sleep
- push to gibhub

*/

// The digital input pin for a clockwise turn of the dial
const int clockwise_pin = 5;
// The digital input pin for a counter-clockwise turn of the dial
const int counter_clockwise_pin = 6;
// The digital input pin for a downward-push of the dial
const int down_push_pin = 10;
// The PWM output pin for the peizo buzzer
const int buzzer_pin = 11;
// The digital output pin for the NADLink signal
const int nadlink_signal_pin = 13;

// global variables to track the input and speaker status
boolean power_is_on = false;
// (there is only a toggle command for speaker, so we have 
// to keep track of their states)
// boolean speaker_a_on = true;
// boolean speaker_b_on = false;

// NAD C 740 address
// The (8 bit) address is transferred using pulse distance encoding 
// with the least signficant bit going first over the wire.
// Afterwards, the bitwise negation is sent. Note: This is where 
// NAD diverges slightly from the NEC spec. NAD does not send 
// exactly the inverted address as the second byte. Instead, 
// typically **1 or 2 bits are not inverted**. This is not a problem, 
// since it is included in the .ir files available at the NAD website.
byte nad_c_740_address_1 = 0x87; // B10000111
byte nad_c_740_address_2 = 0x7C; // B01111100    Note: not the exact inverse of the previous line! See above for explanation.

// NAD C740 commands
byte power_on               = 0x25;
byte power_off              = 0xC8;
byte power_toggle           = 0x80;

byte toggle_speaker_a       = 0xCE;
byte toggle_speaker_b       = 0xCF;

byte switch_input_to_tape_1 = 0x8E;
byte switch_input_to_tape_2 = 0x91;
byte switch_input_to_tuner  = 0x82;
byte switch_input_to_aux    = 0x9B;
byte switch_input_to_video  = 0xC2;
byte switch_input_to_cd     = 0x85;
byte switch_input_to_disc   = 0x89;

byte increase_volume     = 0x88;
byte decrease_volume     = 0x8C;
byte toggle_mute         = 0x94;

// Default Volume
float default_volume_level = 1.1; // standard volume control position (on a scale of 0 to 11, 2 is more than enough)

// Funtion prototypes (where neccesary)
void send_command(byte command, boolean pause_before_and_aftercommand = true);

// Initializing function (runs once on power-up)
void setup() {

  // Setup the pins for input/output
  pinMode(down_push_pin,         INPUT_PULLUP);
  pinMode(clockwise_pin,         INPUT_PULLUP);
  pinMode(counter_clockwise_pin, INPUT_PULLUP);
  pinMode(buzzer_pin,            OUTPUT);
  pinMode(nadlink_signal_pin,    OUTPUT);

  // Set up the interups
  attachInterrupt(down_push_pin,         toggle_power,           FALLING);
  attachInterrupt(clockwise_pin,         change_volume_one_tick, FALLING);

  // initialize serial (for debugging)
  // Serial.begin(9600);
}

void toggle_power(){
//  play_click_sound();
  if(power_is_on){
//    Serial.print("sending OFF commands... ");
    turn_off();
    power_is_on = false;
//    Serial.println("done");
    return;
  } else {
//    Serial.print("sending ON commands... ");
    turn_on();
    power_is_on = true;
//    Serial.println("done");
    return;
  }
}

void change_volume_one_tick(){
  // Our rotary encoder outouts two offset square wave forms
  // via pins A and B. We use a falling signal on the A
  // waveform to trigger this ISR, and then check whether B
  // is high or low, which will tell us the direction of
  // the turn.
  
//  play_click_sound();
  if(digitalRead(counter_clockwise_pin) == 0){
//    Serial.print("sending VOL + command... ");
    send_command(increase_volume, false);
//    Serial.println("done");
  } else {
//    Serial.print("sending VOL - command... ");
    send_command(decrease_volume, false);
//    Serial.println("done");
  }
}

// Main loop (runs continuously after the setup function finishes)
void loop() {

}

void demo(){
  
  delayMicroseconds(1000000*3);
  Serial.println("turning on");
  turn_on();
  
  delayMicroseconds(1000000*3);
  Serial.println("volume up X 10");
  for (int i = 0; i < 10; i++){
    send_command(increase_volume, false);
  }
  
  delayMicroseconds(1000000*3);
  Serial.println("volume down X 10");
  for (int i = 0; i < 10; i++){
    send_command(decrease_volume, false);
  }
  
  delayMicroseconds(1000000*3);
  Serial.println("switching to AirPLay");
  switch_to_airplay();
  
  delayMicroseconds(1000000*3);
  Serial.println("turning off");
  turn_off();
  
}

// The NAD Link uses a slightly modified version of the NEC remote control protocol,
// where 0V represents pulse, and +5V represents flat.

// Pulse
void pulse(int microseconds){
  // 0V (Logical HIGH)
  digitalWrite(nadlink_signal_pin, LOW);
//  PORTD = PORTD & ~port_d_pin_binary();
//  PORTB = PORTB & ~port_b_pin_binary();
  delayMicroseconds(microseconds);
}

// Flat
void flat(int microseconds){
  // Pin 2 +5V (Logical LOW)
//  PORTD = PORTD | port_d_pin_binary();
//  PORTB = PORTB | port_b_pin_binary();
  digitalWrite(nadlink_signal_pin, HIGH);
  delayMicroseconds(microseconds);
}

// Preamble
void command_preamble(){
  pulse(9000); // 9000 μs pulse
  flat(4500);  // 4500 μs flat
}

// Command Terminator
void command_terminator(){
  pulse(560);  //   560 μs pulse
  flat(42020); // 42020 μs flat
}

// sends the repeat signal
static void send_repeat()
{
  pulse(9000); //  9000 μs pulse
  flat(2250);  //  2250 μs flat
  pulse(560);  //   560 μs pulse
  flat(98190); // 98190 μs flat
}

void send_one_bit(){
  pulse(560); //  560 μs pulse
  flat(1690); // 1690 μs flat
}

void send_zero_bit(){
  pulse(560); // 560 μs pulse
  flat(560); //  560 μs flat
}

void send_byte(byte data_byte){
  for (byte mask = B00000001; mask > 0; mask <<= 1) { // iterate through a bit mask
    if (data_byte & mask) {
      send_one_bit();
    }
    else {
      send_zero_bit();    
    }
  }
}

void send_byte_and_inverse(byte data_byte){
  send_byte(data_byte);
  send_byte(~data_byte);
}

// sends a complete command 
// (with preample, two address bytes, 
// two command bytes and the terminator)
void send_command(byte command, boolean pause_before_and_after_command){
  int pause_length_in_ms = 250;
  
  // pause (running commands too close together seems to cause them to get ignored)
  if (pause_before_and_after_command) {
    delayMicroseconds(1000*pause_length_in_ms);
  }
  
  // send preamble signal
  command_preamble();
  
  // send address part 1 and 2
  send_byte(nad_c_740_address_1);
  send_byte(nad_c_740_address_2);

  // send command and inverted command
  send_byte_and_inverse(command);
  
  // send command terminator signal
  command_terminator();
  
  if (pause_before_and_after_command) {
    delayMicroseconds(1000*pause_length_in_ms);
  }
}

void change_volume_to_default() {
  // returns the volume control
  // to zero (no matter what position it was in before)
  send_command(increase_volume, false);
  for(int i = 0; i < (113*default_volume_level/11); ++i) { 
    // 113 repeats of the volume command 
    // is just over a full rotation of the 
    // volume dial on my NAD C740
    send_repeat();
  }
}

void change_volume_to_zero() {
  // sets the volume to the specified volume
  // on a scall from 0 to 11, 11 being the loudest
  // (assumes volume is currently at 0)
  send_command(decrease_volume, false);
  for(int i = 0; i < (113*default_volume_level/11+5); ++i) { 
    // 113 repeats of the volume command 
    // is just over a full rotation of the 
    // volume dial on my NAD C740
    send_repeat();
  }
}



void toggle_speakers_a_b(){
  // toggles the speakers
  // assumes exactly one is on and one is off
  send_command(toggle_speaker_a);
  send_command(toggle_speaker_b);
}

void switch_using(byte input_switch_command){
  // mute
  // send_command(toggle_mute);
  //switch speakers
  toggle_speakers_a_b();
  // switch to passed input
  send_command(input_switch_command);
  // unmute
  // send_command(toggle_mute);
}

// for testing, cycle through all 255 codes
void test_all_codes(){
  for (byte code = 0xFF; code > 0x00; code--){
    Serial.println(code, HEX);
    send_command(code);
    delayMicroseconds(1000000*200);
  }
}

void turn_on(){
  // power up
  send_command(power_on);
  // wait 4s for the amp to power up and turn on the inputs
  delayMicroseconds(1000000*4);
  switch_to_white_noise();
  // volume to default
  change_volume_to_default();
}

void switch_to_airplay() {
  send_command(switch_input_to_video);
}
  
void switch_to_white_noise(){
  send_command(switch_input_to_aux);
}

void turn_off(){
  // volume to zero
  change_volume_to_zero();
  // switch to aux and set speakers A
  switch_to_white_noise();
  // power down
  send_command(power_off);
}
