/*
 * Classroom music teaching aid
 * ATmega328 remote keyboard
 * by shlonkin
 * 
 * The ATMega328 at 8MHz that reads buttons on the keyboard 
 * and sends the data via IR
 * 
 * This code is in the public domain.
 */

#include "IrOutput.h"

#define irCode 0xAC

// this is the pin mapping for the 15 notes
const byte notes[15] = {14,5,6,19,17,15,16,18,7,8,2,1,4,0,3};

unsigned long lastEventTime;

void setup() {
  // set up key pins
  for(byte i=0; i<15; i++){
    pinMode(notes[i], INPUT_PULLUP);
  }
  
  // set up IR
  irBegin();

  lastEventTime = millis();
}

void loop() {
  // Check each key
  for(byte i=0; i<15; i++){
    if(digitalRead(notes[i]) == LOW){
      keyPressed(i);
      lastEventTime = millis();
    }
  }

  // if nothing happens for 5 minutes, go to sleep
  if(millis() - lastEventTime > 300000UL){
    goToSleep();
  }
}

void keyPressed(byte n){
  // start by transmitting the code and note number
  irSend(irCode);
  irSend(n);
  // as long as the key is pressed, transmit the note number every 50ms
  // sending a byte might take around 30ms
  while(digitalRead(notes[n]) == LOW){
    irSend(n);
    delay(20);
  }
  // now the key is released
  irSend(0xFF);
  // for debounce
  delay(50);
}

void goToSleep(){
  // enter power down sleep mode
  // in this mode everything stops
  SMCR = (1<<SM1)|(1<<SE);

  // enable int1 which is pin 3
  // trigger is low level
  attachInterrupt(1, wakeUp, LOW);
  // good night
  __asm__ __volatile__ ( "sleep" "\n\t" :: );
}

void wakeUp(){
  // turn off the sleep mode
  SMCR = 0;
  detachInterrupt(1);
}

