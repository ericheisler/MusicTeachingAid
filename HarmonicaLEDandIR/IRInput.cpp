/*
 * A simple library for receiving IR input.
 * The timing is similar to the NEC protocol, but the
 * structure is different.
 * 
 * Input is on pin 2 and uses the int0 interrupt.
 * 
 * This code is in the public domain.
 */
 
#include "IRInput.h"

#define irPin 2
#define irCode 0xAC
#define bitthreshold 1500
#define startthreshold 10000
#define newFrameThreshold 15000

volatile bool headerReceived;
volatile unsigned long irStartTime;
volatile byte bitIndex, tmpByte;
volatile unsigned long irStopTime;
byte irInputBuffer[8] = {0,0,0,0,0,0,0,0};
byte irBufferIndex;

// falling edge on pin 2 triggers interrupt
ISR(INT0_vect){
  irStopTime = micros();
  if(irStopTime - irStartTime > newFrameThreshold){
    headerReceived = false;
    irStartTime = irStopTime;
  }else if(!headerReceived){
    // don't worry about header unless there is some problem
    irStartTime = irStopTime;
    headerReceived = true;
    tmpByte = 0;
    bitIndex = 0;
  }else{
    // write one bit to the temp byte after shifting it left
    // when eight bits are written, store the byte in the buffer
    tmpByte <<= 1;
    bitIndex++;
    if(irStopTime-irStartTime > bitthreshold){
      tmpByte++;
    }
    irStartTime = irStopTime;
    
    if(bitIndex == 7){
      // a byte has been received
      // put the newest byte in the [0] space in the buffer
      // if the buffer is full, discard the oldest byte
      byte i = irBufferIndex;
      while(i > 0){
        irInputBuffer[i] = irInputBuffer[i-1];
        i--;
      }
      irInputBuffer[0] = tmpByte;
      irBufferIndex++;
      if(irBufferIndex == 8){
        irBufferIndex = 7;
      }
      tmpByte = 0;
      bitIndex = 0;
    }
  }
}

void irBegin(){
  headerReceived = false;
  irBufferIndex = 0;
  pinMode(irPin, INPUT_PULLUP);
  EICRA |= (1<<ISC01); // falling edge on pin 2 triggers interrupt
  EIMSK |= (1<<INT0);
  sei();
}

/*
 * The number of unread bytes in the buffer
 */
byte irAvailable(){
  return(irBufferIndex);
}

/*
 * Return the oldest available byte in the buffer.
 * The returned byte is removed from the buffer.
 */
byte irRead(){
  if(irBufferIndex > 0){
    irBufferIndex--;
    return irInputBuffer[irBufferIndex];
  }
  return 0;
}

