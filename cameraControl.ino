/*
  Send a Start/Stop Recording command to the LANC port of a video camera.
  Tested with a Canon XF300 camcorder
  This code requires a simple interface see http://micro.arocholl.com
  Feel free to use this code in any way you want.

  Comprehensive LANC info: www.boehmel.de/lanc.htm

  "LANC" is a registered trademark of SONY.
  CANON calls their LANC compatible port "REMOTE".

  2011, Martin Koch
  http://controlyourcamera.blogspot.com/2011/02/arduino-controlled-video-recording-over.html

  2020, Remmelt Pit
*/

#include <Arduino.h>

#define cmdPin 10
#define lancPin 9
#define recButton 11
#define blackButton A0

// Duration of one LANC bit is 104ms.
// Writing to the digital port takes about 8 microseconds so only 96 microseconds are left till the end of each bit
const int bitDuration = 96;
const int debounceDelayMs = 100;
const int cmdRepeatCount = 5;
const int initialPauseMs = 5000;

// "Off" states:
int fnState = 1;
int recState = 1;

void setup() {
  pinMode(lancPin, INPUT); //listens to the LANC line
  pinMode(cmdPin, OUTPUT); //writes to the LANC line

  pinMode(recButton, INPUT_PULLUP);
  pinMode(blackButton, INPUT_PULLUP);

  digitalWrite(cmdPin, LOW); //set LANC line to +5V
  delay(initialPauseMs); //Wait for camera to power up completely
}

void writeValue(byte value) {
  for (byte i = 0, mask = 1; i < 8; i++, mask = mask << 1u) {
    digitalWrite(cmdPin, value & mask);
    delayMicroseconds(bitDuration);
  }
}

void writeCommand(byte command[], int nrOfBytesToWrite) {
  // Write the command a number of times, according to the spec
  for (byte c = 0; c < cmdRepeatCount; c++) {
    while (pulseIn(lancPin, HIGH) < 5000) {
      //"pulseIn, HIGH" catches any 0V TO +5V TRANSITION and waits until the LANC line goes back to 0V
      //"pulseIn" also returns the pulse duration so we can check if the previous +5V duration was long enough (>5ms) to be the pause before a new 8 byte data packet
      //Loop till pulse duration is >5ms
    }

    for (byte c = 0; c < nrOfBytesToWrite; c++) {
      delayMicroseconds(bitDuration);  //wait START bit duration
      writeValue(command[c]);
      digitalWrite(cmdPin, LOW);

      if (nrOfBytesToWrite - c > 1) {
        delayMicroseconds(10);
        while (digitalRead(lancPin)) {
          //Loop as long as the LANC line is +5V during the stop bit
        }
      }
    }
  }
}

// https://github.com/imaginevision/Z-Camera-Doc/blob/master/E2/protocol/lanc.md
void REC() {
  const byte v[2] = {0x18, 0x33};
  writeCommand(v, 2);
}

void FN() {
  const byte v[3] = {0x55, 0x54, 0x10};
  writeCommand(v, 3);
}

void loop() {
  if (!digitalRead(recButton)) {
    if (recState == 1) {
      REC(); //send REC command to camera
    }
    recState = 0;
    delay(debounceDelayMs);
  } else {
    recState = 1;
  }
  if (!digitalRead(blackButton)) {
    if (fnState == 1) {
      FN(); //send FN command to camera
    }
    fnState = 0;
    delay(debounceDelayMs);
  } else {
    fnState = 1;
  }
}
