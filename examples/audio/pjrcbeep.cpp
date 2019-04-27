/*
lib_deps =
  AudioALS
  161

lib_ignore =
  70
*/
#include <Audio.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <SerialFlash.h>

AudioSynthWaveform    waveform1;
AudioOutputAnalogStereo  audioOutput;
AudioConnection       patchCord1(waveform1, 0, audioOutput, 0);
AudioConnection       patchCord2(waveform1, 0, audioOutput, 1);

int count=1;

void setup() {
  AudioMemory(10);
  Serial.begin(115200);
  waveform1.begin(WAVEFORM_SINE);
  delay(1000);
}

void loop() {
  Serial.print("Beep #");
  Serial.println(count);
  count = count + 1;
  waveform1.frequency(440);
  waveform1.amplitude(0.9);
  delay(250);
  waveform1.amplitude(0);
  delay(1750);
}

