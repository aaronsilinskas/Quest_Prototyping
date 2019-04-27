/*
lib_deps =
  AudioALS
  161

lib_ignore =
  70
*/
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

const uint8_t PIN_SD_CS = 2;
const uint8_t PIN_BUTTON = 11;

AudioPlaySdWav           playWav1;
AudioOutputAnalogStereo  audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
File root;

void printDirectory(File dir, int numTabs)
{
    while (true)
    {

        File entry = dir.openNextFile();
        if (!entry)
        {
            // no more files
            break;
        }
        for (uint8_t i = 0; i < numTabs; i++)
        {
            Serial.print('\t');
        }
        Serial.print(entry.name());
        if (entry.isDirectory())
        {
            Serial.println("/");
            printDirectory(entry, numTabs + 1);
        }
        else
        {
            // files have sizes, directories do not
            Serial.print("\t\t");
            Serial.println(entry.size(), DEC);
        }
        entry.close();
    }
}

void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  if (!playWav1.play(filename)) {
      Serial.println("Unable to play...");
  }
  Serial.print("Length (millis): ");

  // A brief delay for the library read WAV info
  delay(50);

  // Simply wait for the file to finish playing.
  while (playWav1.isPlaying()) {
    // uncomment these lines if you audio shield
    // has the optional volume pot soldered
    //float vol = analogRead(15);
    //vol = vol / 1024;
    // sgtl5000_1.volume(vol);
  }
  Serial.println("Done playing.");
}

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    Serial.println("Allocating audio memory...");
    AudioMemory(16);

    Serial.print("Initializing SD card...");

    if (!SD.begin(PIN_SD_CS))
    {
        Serial.println("initialization failed!");
        while (1)
            ;
    }
    Serial.println("initialization done.");

    root = SD.open("/");

    printDirectory(root, 0);

    Serial.println("done!");

    pinMode(PIN_BUTTON, INPUT_PULLUP);
}

void loop()
{
    // nothing happens after setup finishes.
    if (digitalRead(PIN_BUTTON) == 0) {
        playFile("TEST/T03.WAV");
        delay(500);
    }

    playWav1.update();
}
