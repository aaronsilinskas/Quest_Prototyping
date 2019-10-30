/* read-analog.cpp Example sketch for recording analog signals
 *
 */
#include <Arduino.h>

const int PIN_ANALOG = A0;

#define SAMPLE_SIZE 100

int samples[SAMPLE_SIZE];

void setup()
{
    while (!Serial)
    {
    };

    Serial.begin(9600);
}

void loop()
{
    static int sample = 0;

    if (sample >= SAMPLE_SIZE) {
        int minSample = 1024;
        int maxSample = 0;
        long averageSample = 0;
        for (int i = 0; i < SAMPLE_SIZE; i++) {
            int nextSample = samples[i];
            minSample = min(minSample, nextSample);
            maxSample = max(maxSample, nextSample);
            averageSample += nextSample;
        }
        averageSample /= SAMPLE_SIZE;

        Serial.print(minSample);
        Serial.print(" ");
        Serial.print(averageSample);
        Serial.print(" ");
        Serial.println(maxSample);

        sample = 0;
    }
    samples[sample] = analogRead(PIN_ANALOG);
    sample ++;

    delay(1);
}
