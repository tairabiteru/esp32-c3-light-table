/**
 * This file is a program for an ESP32-C3 microcontroller, detailing a light brightness
 * control system for a light table at the Northville District Library.
 */
#include <Arduino.h>


// Defines minimum and maximum ADC values. Our potentiometer bottoms out
// at around 8-10, so we choose a value slightly above that as our maximum.
#define MIN_ADC 15
#define MAX_ADC 4095


// Output pin for the LEDs.
#define OUTPUT_PIN 3


// Input pin for the ADC for the potentiometer.
#define ADC 2


// The length of time in milliseconds after which the system will go to sleep.
// Define as 1800 seconds, or 30 minutes.
#define TIMEOUT_MS 1800000


// The number of times to sample the potentiometer for each reading.
// This is explained more a bit further down.
#define POT_SAMPLES 100


// Uncomment to enable serial comms, useful for debugging.
// #define DO_SERIAL


// Global variable stores the last time a change was recorded on the potentiometer.
// This lets us keep track of the last time it was turned for sleep.
uint64_t last_change = millis();

// Stores the last value obtained from the ADC.
int last_out = 0;



/**
 * Read a value from the potentiometer.
 * 
 * Each time we want to read the potentiometer, we read it POT_SAMPLES times, and then
 * average the values together. Potentiometers are not perfect, and will very quickly
 * bounce between a range of values which can cause erratic behavior from the lights if
 * this value is used to set a brightness level. By sampling many times and averaging,
 * we get a much more "stable" reading from the ADC.
 * 
 * @return float corresponding to the average of the last 'POT_SAMPLES' reading(s). 
 */
float read_pot() {
  float sum = 0;

  for (int i=0; i<POT_SAMPLES; i++) {
    sum += analogRead(ADC);
  }
  return sum / POT_SAMPLES;
}


/**
 * Arduino framework setup function.
 * 
 * The Arduino framework setup function is run once when the MCU boots, and then is
 * never run again. As its name implies, it is used to set up everything.
 */
void setup() {
  // Do serial if it is defined.
  #ifdef DO_SERIAL
  Serial.begin(115200);
  Serial.println("Ready.");
  #endif

  // Setup the output pin for...well, output.
  pinMode(OUTPUT_PIN, OUTPUT);
  // Set duty cycle frequency to 5kHz. This helps prevent flickering.
  analogWriteFrequency(5000);
}


/**
 * Arduino framework loop function.
 * 
 * The Arduino framework loop function is run continuously after the setup function,
 * and repeats itself forever. This forms the core functionality of what the MCU must
 * do.
 */
void loop() {
  // Read the current value from the potentiometer.
  float val = read_pot();

  // This stores the value to be written out to the LEDs.
  // It ranges from 0 (off) to 255 (max brightness).
  int out = 0;

  // If the value read is greater than the MIN_ADC value we defined,
  // We perform some math to find the appropriate 0-255 value for the lights.
  if (val > MIN_ADC) {
    out = 255 * (val / MAX_ADC);
  }
  // If the value is equal or lower, then we don't care. 'out` is initialized as
  // 0, which is the value it should be if the above condition is false.
  
  // Do serial comms if defined.
  #ifdef DO_SERIAL
  Serial.println(millis() - last_change);
  Serial.println(out);
  Serial.println(last_out);
  Serial.println();
  #endif

  // Check to see how long it's been since the last change,
  // and see if that time exceeds the timeout we defined.
  if (((millis() - last_change) > TIMEOUT_MS)) {
    // If it does, turn the lights off.
    analogWrite(OUTPUT_PIN, 0);

    // Then check to see if the value has changed:
    if (abs(out - last_out) > 10) {
      // If it has, update the last recorded change.
      last_out = out;
      last_change = millis();

      // The next time the loop runs after a change, the condition on line 115
      // will be false, and the else block will execute instead.
    }

  // If the if statement is false, we simply write the brightness value out to
  // the LED pin.
  } else {
    analogWrite(OUTPUT_PIN, out);
  }

  // Shot delay of 1 ms to prevent the MCU from panting.
  delay(1);
}