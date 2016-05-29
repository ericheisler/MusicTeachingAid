/*
 * Classroom music teaching aid
 * Teensy3.2 code
 * by shlonkin
 * 
 * This makes use of Teensyduino libraries, in particular the Audio libraries.
 * See the corresponging files for their licence info.
 * This code is in the public domain.
 * 
 * This code does:
 * - Normally it is listening to mic input and determining note frequency
 *   via the AudioAnalyzeNoteFrequency library.
 * - When a note is detected, the note number(1-15) is sent by Serial to
 *   the ATMega to turn on the LEDs. When the note is no longer playing, 
 *   the number 0 is sent to turn LEDs off.
 * - When Serial input is recieved it means a key was pressed on the remote.
 *   The corresponding sound file is played and LEDs are turned on. The input
 *   will come every 100ms until the key is released at which point the sound
 *   will fade out and the LEDs will turn off.
 * - When the record button is pressed, incoming note and timing data are
 *   stored in a MIDI type file(exact specification not decided yet).
 * - When the play button is pressed, the recorded file is played back.
 * 
 */

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Bounce.h>

#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// a mapping of note numbers to sample filenames
const char noteFiles[15][7] = {"G4.WAV", "A5.WAV", "B5.WAV", "C5.WAV", "D5.WAV"
, "E5.WAV", "F5.WAV", "G5.WAV", "A6.WAV", "B6.WAV", "C6.WAV", "D6.WAV", "E6.WAV"
, "F6.WAV", "G6.WAV"};

const int myInput = AUDIO_INPUT_MIC;

// The Audio components
AudioInputI2S audioInput;
AudioPlayMemory samplePlayer;
AudioAnalyzeNoteFrequency noteFreq;
AudioOutputI2S audioOutput;
AudioPlaySdWav noteSample;

// Connect the input
AudioConnection c1(audioInput, 0, noteFreq, 0);
// and the output
AudioConnection c2(samplePlayer, 0, audioOutput, 0);

AudioControlSGTL5000 audioShield;

// Bounce objects to read buttons (pins 2 and 3)
Bounce playButton = Bounce(2, 15);
Bounce recordButton = Bounce(3, 15);

void setup() {
  
  AudioMemory(30);

  // Enable the audio shield and set the output volume.
  audioShield.enable();
  audioShield.inputSelect(AUDIO_INPUT_MIC);
  audioShield.volume(0.5);

  // init noteFreq
  noteFreq.begin(.15);

  // use Serial1
  Serial1.begin(9600);
  // Wait till the ATMega chip is ready.
  // It will send a character, respond with one.
  while(Serial1.available() < 1);
  Serial1.write('A');
  Serial1.clear();

  // set up the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here and blink LED
    while (1) {
      Serial1.write(0);
      delay(500);
      Serial1.write(0xFF);
      delay(1000);
    }
  }
}

/*
 * The main loop checks for input from serial and the
 * frequency detector. If there is serial input, go into 
 * keyboard mode. If there is a note detected, go into
 * mic mode. The two can't be done simultaneously.
 */
void loop() {
  if(Serial1.available() > 0){
    keyboardMode();
  }
  
  if (noteFreq.available()) {
    micMode();
  }
    
}

void keyboardMode(){
  byte in = Serial1.read();
  bool stillPlaying = false;
  if(in < 15){
    // play the note
    startSample(in);
    stillPlaying = true;
  }
  while(stillPlaying){
    if(!noteSample.isPlaying()){
      noteSample.play(noteFiles[in]);
    }
    if(Serial1.available() > 0){
      in = Serial1.read();
      if(in == 0xFF){
        stopSample();
        stillPlaying = false;
      }else if(in <14){
        // this should only happen if the note changed
        noteSample.stop();
        noteSample.play(noteFiles[in]);
      }
    }
  }
  // At this point the note is off. Exit keyboard mode.
}

void micMode(){
  byte note = fToNote(noteFreq.read());
  if(note > 14){
    // the frequency was not in range
    return;
  }
  Serial1.write(note);
  // do some simple filtering
  byte burps = 0;
  while(burps < 5){
    burps = 0;
    for(byte i=0; i<10; i++){
      if(note != fToNote(noteFreq.read())){
        burps++;
      }
      delay(10);
    }
    noteFreq.available(); //Just to clear the flag
  }
  Serial1.write(0xFF);
}

void startSample(byte n){
  // increase the volume over 250ms
  audioShield.volume(0.01);
  noteSample.play(noteFiles[n]);
  for(int i=1; i<=25; i++){
    audioShield.volume(0.02*i);
    delay(10);
  }
}

void stopSample(){
  // decrease the volume over 250ms
  for(int i=25; i>0; i--){
    if(noteSample.isPlaying()){
      audioShield.volume(0.02*i);
      delay(10);
    }
  }
  noteSample.stop();
  audioShield.volume(0.5);
}

byte fToNote(float f){
  // returns 15 if the note is out of range
  if(f < 370){
    return(15);
  }else if(f < 415){
    return(0);
  }else if(f < 466){
    return(1);
  }else if(f < 508){
    return(2);
  }else if(f < 554){
    return(3);
  }else if(f < 622){
    return(4);
  }else if(f < 680){
    return(5);
  }else if(f < 740){
    return(6);
  }else if(f < 830){
    return(7);
  }else if(f < 932){
    return(8);
  }else if(f < 1015){
    return(9);
  }else if(f < 1108){
    return(10);
  }else if(f < 1244){
    return(11);
  }else if(f < 1358){
    return(12);
  }else if(f < 1480){
    return(13);
  }else if(f < 1661){
    return(14);
  }
  return(15);
}


