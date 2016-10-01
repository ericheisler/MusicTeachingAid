/*
 * A simple library for transmitting IR output.
 * The timing is similar to the NEC protocol, but the
 * structure is different.
 * 
 * Output is on pin 29 and uses timer OC1A.
 * 
 * This code is in the public domain.
 */
#ifndef IROUT_h
#define IROUT_h

#include "arduino.h"

// this is very simple
void irBegin(void);
void irSend(byte b);

// private
void irSendHeader();
void irSendByte(byte output);
void irSendStopBit();

#endif
