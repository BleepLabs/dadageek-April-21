/*
  Bleep Base example
  Using LEDs

  The bleep base vs has two addressable LEDs and one standard LED on the Teensy 4.1
*/

//A special library must be used to communicate with the two ws2812 addressable LEDs on the board
// The standard LED libraries, like fastLED and adafruits neopixel, cause problems with the audio code so this version is used
// except for these line:
#define num_of_leds 20 //increase for external LEDs
float max_brightness = .1; //change this to increase the max brightness of the LEDs. 1.0 is crazy bright
//everything else can be left alone

#include <WS2812Serial.h> //include the code from this file in our sketch  https://github.com/PaulStoffregen/WS2812Serial/archive/refs/heads/master.zip
#define led_data_pin 29
byte drawingMemory[num_of_leds * 3];
DMAMEM byte displayMemory[num_of_leds * 12];
WS2812Serial LEDs(num_of_leds, displayMemory, drawingMemory, led_data_pin, WS2812_GRB);

//then we can get back to the normal initialization section

unsigned long current_time;
unsigned long prev_time[8]; //an array of 8 variables, all named prev_time

//float is a floating point variable. They are the only kind of numbers  that can hold decimals.
// We don't use them for everything as they take a little more time to compute than integers and longs
float rainbow;
float lfo1 = 1;
float lfo2;
int lfo1_latch;
float pot_reading[8];

void setup() {

  LEDs.begin(); //must be done in setup for the addressable LEDs to work.
  //here is the basic way of writing to the LEDs.
  LEDs.setPixelColor(0, 0, 0, 0); //(LED number, red level, green level, blue level). All levels are 0-255
  LEDs.setPixelColor(1, 0, 0, 0);
  LEDs.show(); //send these values to the LEDs

  analogWriteResolution(10); //Teensy 4.1 can do 0-1023 levels of PWM output

}

void loop() {
  current_time = millis();

  if (current_time - prev_time[0] > 10) {
    prev_time[0] = current_time;

    if (lfo1_latch == 1) { //== is compare, not set equal to
      lfo1 += 5; //same as lfo=lfo+5;
    }
    if (lfo1_latch == 0) {
      lfo1 -= 3; //same as lfo=lfo-3;
    }

    if (lfo1 < 1) {
      lfo1 = 1;
      lfo1_latch = 1;
    }
    if (lfo1 > 500) {
      lfo1 = 500;
      lfo1_latch = 0;
    }

    //analogWrite( pin, value) Pin 13 is attached to an LED on pretty much every Arduino board
    // but on the Teensy 4.x it it can do PWM aka analogEWrite
    // 10bit out means 0-1023 levels
    analogWrite(13, lfo1);

  }


  if (current_time - prev_time[1] > 33) { //33 milliseconds is about 30 Hz, aka 30 fps
    prev_time[1] = current_time;

    rainbow += 0.0025;
    if (rainbow > 1.0) {
      rainbow -= 1.0;
    }

    pot_reading[0] = analogRead(A10) / 1023.0;
    pot_reading[1] = 1.0 - (analogRead(A11) / 1023.0);
    pot_reading[2] = 1.0 - (analogRead(A12) / 1023.0);

    //there's another function in this sketch bellow the loop which makes it easier to control the LEDs
    // more info bellow the loop

    //when doing math on floats, be sure to add a ".0" so it works properly
    // lfo goes from 0-1023 but we need it to go from 0.0-1.0
    lfo2 = lfo1 / 1023.0;
    set_LED(0, pot_reading[0], pot_reading[1], pot_reading[2]); //(led to change, hue,saturation,brightness)
    set_LED(1, rainbow, 1, 1);
    LEDs.show(); //send these values to the LEDs
  }




} //loop is over

//This function is a little different than you might see in other libraries but it works pretty similar
// instead of 0-255 you see in other libraries this is all 0-1.0
// you can copy this to the bottom of any code as long as the declarations at the top in "led biz" are done

//set_LED(led to change, hue,saturation,value aka brightness)
// hue - 0 is red, then through the ROYGBIV to 1.0 as red again
// saturation - 0 is fully white, 1 is fully colored.
// value - 0 is off, 1 is the value set by max_brightness
// (it's not called brightness since, unlike in photoshop, we're going from black to fully lit up

//based on https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both

void set_LED(int pixel, float fh, float fs, float fv) {
  byte RedLight;
  byte GreenLight;
  byte BlueLight;

  byte h = fh * 255;
  byte s = fs * 255;
  byte v = fv * max_brightness * 255;

  h = (h * 192) / 256;  // 0..191
  unsigned int i = h / 32;   // We want a value of 0 thru 5
  unsigned int f = (h % 32) * 8;   // 'fractional' part of 'i' 0..248 in jumps

  unsigned int sInv = 255 - s;
  unsigned int fInv = 255 - f;
  byte pv = v * sInv / 256;  // pv will be in range 0 - 255
  byte qv = v * (256 - s * f / 256) / 256;
  byte tv = v * (256 - s * fInv / 256) / 256;

  switch (i) {
    case 0:
      RedLight = v;
      GreenLight = tv;
      BlueLight = pv;
      break;
    case 1:
      RedLight = qv;
      GreenLight = v;
      BlueLight = pv;
      break;
    case 2:
      RedLight = pv;
      GreenLight = v;
      BlueLight = tv;
      break;
    case 3:
      RedLight = pv;
      GreenLight = qv;
      BlueLight = v;
      break;
    case 4:
      RedLight = tv;
      GreenLight = pv;
      BlueLight = v;
      break;
    case 5:
      RedLight = v;
      GreenLight = pv;
      BlueLight = qv;
      break;
  }
  LEDs.setPixelColor(pixel, RedLight, GreenLight, BlueLight);
}
