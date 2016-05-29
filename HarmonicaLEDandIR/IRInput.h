/*
 * A simple library for receiving IR input.
 * The timing is similar to the NEC protocol, but the
 * structure is different.
 * 
 * Input is on pin 2 and uses the int0 interrupt.
 * 
 * This code is in the public domain.
 */
#ifndef IR_h
#define IR_h

#include "arduino.h"

// this is very simple
void irBegin(void);
byte irAvailable();
byte irRead();

#endif
