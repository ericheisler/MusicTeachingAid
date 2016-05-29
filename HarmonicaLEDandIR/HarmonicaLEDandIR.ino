/*
 * Classroom music teaching aid
 * ATmega328 LED and IR controller code
 * by shlonkin
 * 
 * The ATMega328 at 8MHz that controls LEDs and receives IR from 
 * the remote. It communicates with the Teensy via Serial.
 * 
 * This code is in the public domain.
 */

#include "IRInput.h"

#define irCode 0xAC

// this is the pin mapping for the 15 notes
// will be updated once the wires are in place
const byte notes[15] = {5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};

void setup() {
  // set up LED pins
  for(int i=0; i<15; i++){
    pinMode(notes[i], OUTPUT);
    digitalWrite(notes[i], LOW);
  }

  // set up IR
  irBegin();

  // Start serial, spam ack request until ack is received
  Serial.begin(9600);
  delay(500); // wait a moment to let the Teensy get ready
  while (Serial.available() <= 0) {
    Serial.print('A');
    delay(300);
  }
  Serial.read(); // clear the ack
  
}

/*
 * Listen for input from both IR and serial. 
 * If IR comes, enter keyboardIn mode.
 * If serial comes, enter micIn mode.
 * Both modes cannot be done simultaneously
 */
void loop() {
  // 
  if(irAvailable() > 0){
    keyboardInMode();
  }
  if(Serial.available() > 0){
    micInMode();
  }
}

/*
 * In keyboardIn mode:
 * First a code byte is received. If the code is wrong, discard the byte.
 * If it is correct, continue.
 * The second byte contains the number of the note to be played.
 * As long as the key is held down the second byte will be sent
 * every 200ms. When the key is released, a 0xFF byte will be sent.
 * The 0xFF means to turn the note off and return.
 * If the second byte is not received for 250ms, something went wrong
 * so turn off the note and return.
 */
void keyboardInMode(){
  byte in = irRead();
  if(in != irCode){
    // the byte has been discarded, just return
    return;
  }
  // wait for the next byte to arrive
  unsigned long timeout = millis();
  while(irAvailable() == 0){
    if(millis() - timeout > 250){
      // It took too long. Just start over.
      return;
    }
  }
  // If a valid note number is received, tell the Teensy to play
  // it and light up the LEDs.
  in = irRead();
  if(in > 14){
    // huh?
    return;
  }
  byte thisNote = in;
  bool transition = false;
  Serial.write(thisNote);
  turnOn(thisNote);
  
  // be careful with this infinite loop
  while(true){
    timeout = millis();
    while(irAvailable() == 0){
      if(millis() - timeout > 250){
        // It took too long. Turn off things and return
        Serial.write(0xFF);
        turnOff();
        return;
      }
    }
    in = irRead();
    if(in == 0xFF){
      // turn things off and return
      Serial.write(0xFF);
      turnOff();
      return;
    }
    if(transition){
      transition = false;
      thisNote = in;
      Serial.write(thisNote);
      turnOn(thisNote);
    }
    if(in == irCode){
      // that means a new note was played
      // transition to that note
      transition = true;
    }
    // In any other case just keep on playing.
  }
}

/*
 * In mic in mode:
 * A byte has been sent from the Teensy.
 * If it is a valid note number, turn on the matching LEDs.
 * If it is 0xFF, turn LEDs off.
 * 
 */
void micInMode(){
  byte in = Serial.read();
  if(in > 14){
    // something is funky
    return;
  }
  turnOn(in);
  bool stillOn = true;
  while(stillOn){
    // Now just wait until the note is turned off.
    // Although it could just return now and do something else
    // while it waits, it is not supposed to.
    if(Serial.available() > 0){
      in = Serial.read();
      if(in == 0xFF){
        turnOff();
        stillOn = false;
      }else if(in < 15){
        // if the note changed, change the light
        turnOn(in);
      }else{
        // something is funky
        turnOff();
        stillOn = false;
      }
    }
  }
  // At this point things should be off.
}

void turnOn(byte n){
  digitalWrite(notes[n], HIGH);
}

void turnOff(){
  for(byte i=0; i<15; i++){
    digitalWrite(notes[i], LOW);
  }
}

