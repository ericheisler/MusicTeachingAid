/*
 * A simple library for transmitting IR output.
 * The timing is similar to the NEC protocol, but the
 * structure is different.
 * 
 * Output is on pin 9 and uses timer OC1A.
 * 
 * This code is in the public domain.
 */
 
#include "IROutput.h"

#define irPin 9
#define headerOn 9000
#define headerOff 4500
#define pulseLength 562
#define zeroBit 562
#define oneBit 1687
#define frameInterval 16000
#define txOn TCCR1A = (1<<COM1A0)
#define txOff TCCR1A = 0

unsigned long lastSendTime;

void irBegin(){
  pinMode(irPin, OUTPUT);
  
  // set up timer 1A for a 38kHz carrier
  TCCR1A = 0;
  TCCR1B = (1<<WGM12)|(1<<CS10);
  OCR1AH = 0;
  OCR1AL = 104;
  OCR1AH = 0;
  OCR1AL = 104;

  lastSendTime = 0;
}

void irSend(byte b){
  // Wait till it is OK to send.
  // There is a minimum wait time between separately sent bytes
  while(micros() - lastSendTime < frameInterval);
  irSendHeader();
  irSendByte(b);
  irSendStopBit();
  lastSendTime = micros() - 1000;
}

/*
* Transmits the header pulse
*/
void irSendHeader(){
  // header
  txOn;
  delayMicroseconds(headerOn);
  txOff;
  digitalWrite(irPin, LOW);
  delayMicroseconds(headerOff);
}

/*
* Transmits the data
*/
void irSendByte(byte output){
  for(int8_t i=7; i>=0; i--){
    txOn;
    delayMicroseconds(pulseLength);
    txOff;
    digitalWrite(irPin, LOW);
    if(output & (1<<i)){
      delayMicroseconds(oneBit);
    }else{
      delayMicroseconds(zeroBit);
    }
  }
}

/*
* Transmits the stop bit
*/
void irSendStopBit(){
  // stop bit
  txOn;
  delayMicroseconds(pulseLength);
  txOff;
  digitalWrite(irPin, LOW);
  delay(1);
}

