#include "effect_tape_delay.h" // this needs to be before the other audio code 

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform2;      //xy=93,662
AudioSynthWaveform       waveform1;      //xy=97,613
AudioSynthWaveform       waveform4;      //xy=101,768
AudioSynthWaveform       waveform3;      //xy=107,733
AudioAnalyzePeak         peak2;          //xy=284,631
AudioAnalyzePeak         peak1;          //xy=287,591
AudioMixer4              mixer1;         //xy=358,810
AudioEffectTapeDelay         delay1;         //xy=358,927
AudioMixer4              mixer2;         //xy=523,712
AudioOutputI2S           i2s1;           //xy=694,726
AudioConnection          patchCord1(waveform2, peak2);
AudioConnection          patchCord2(waveform1, peak1);
AudioConnection          patchCord3(waveform4, 0, mixer1, 1);
AudioConnection          patchCord4(waveform4, 0, mixer2, 1);
AudioConnection          patchCord5(waveform3, 0, mixer1, 0);
AudioConnection          patchCord6(waveform3, 0, mixer2, 0);
AudioConnection          patchCord7(mixer1, delay1);
AudioConnection          patchCord8(mixer1, 0, mixer2, 2);
AudioConnection          patchCord9(delay1, 0, mixer1, 3);
AudioConnection          patchCord10(mixer2, 0, i2s1, 0);
AudioConnection          patchCord11(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=539,590
// GUItool: end automatically generated code

#define DELAY_SIZE 20000 //size in 2xintegers
int16_t tape_delay_bank[DELAY_SIZE]; //int16_t is a more specific way of saying integer

//A special library must be used to communicate with the two ws2812 addressable LEDs on the board
// The standard LED libraries, like fastLED and adafruits neopixel, cause problems with the audio code so this version is used
// except for these line:
#define num_of_leds 2 //increase for external LEDs
float max_brightness = .1; //change this to increase the max brightness of the LEDs. 1.0 is crazy bright

//everything else can be left alone
#include <WS2812Serial.h> //include the code from this file in our sketch  https://github.com/PaulStoffregen/WS2812Serial/archive/refs/heads/master.zip
#define led_data_pin 29
byte drawingMemory[num_of_leds * 3];
DMAMEM byte displayMemory[num_of_leds * 12];
WS2812Serial LEDs(num_of_leds, displayMemory, drawingMemory, led_data_pin, WS2812_GRB);

//this reades the buttons and can be left alone
#include <Bounce2.h>
#define NUM_BUTTONS 8
const int BUTTON_PINS[NUM_BUTTONS] = {30, 31, 32, 33, 34, 35, 36, 37};
Bounce * buttons = new Bounce[NUM_BUTTONS];
#define BOUNCE_LOCK_OUT

//starts at midi note 12, C0 https://newt.phys.unsw.edu.au/jw/notes.html
PROGMEM const static float chromatic[121] = {16.3516, 17.32391673, 18.35405043, 19.44543906, 20.60172504, 21.82676736, 23.12465449, 24.499718, 25.95654704, 27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192, 8869.845359, 9397.273811, 9956.06479, 10548.08321, 11175.30488, 11839.82309, 12543.8556, 13289.75207, 14080.00185, 14917.24233, 15804.26772, 16744.03838};

//then you can declare any variables you want.
unsigned long current_time;
unsigned long prev_time[8]; //an array of 8 variables all named "prev_time"
float lfo[4];
float rainbow;
int pcell1, smooth1;
float freq[5];
int dt; 
void setup() {

  LEDs.begin(); //must be done in setup for the addressable LEDs to work.
  //here is a basic way of writing to the LEDs.
  LEDs.setPixelColor(0, 0, 0, 0); //(LED number, red level, green level, blue level). All levels are 0-255
  LEDs.setPixelColor(1, 0, 0, 0);
  LEDs.show(); //send these values to the LEDs

  analogReadAveraging(64); //take lots of readings and average them each time we do analogRead. This is one stage of smoothing

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  ); //setup the bounce instance for the current button
    buttons[i].interval(10);  // interval in milliseconds. How long after the first cahnge will ignore noise
  }

  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // The Teensy 4.1 hasalmost 2000 block of memory
  // It's usually the delay and reverb that hog it.
  AudioMemory(100);

  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.inputSelect(AUDIO_INPUT_LINEIN); //Tell it what input we want to use. Not necessary is you're not using the ins
  sgtl5000_1.lineInLevel(10); //The volume of the input. 0-15 with 15 bing more amplifications
  //sgtl5000_1.inputSelect(AUDIO_INPUT_MIC);
  //sgtl5000_1.micGain(13); //0 - 63bd of gain.

  //headphone jack output volume. Goes from 0.0 to 1.0 but a 100% signal will clip over .8 or so.
  // For headphones it's pretty loud at .4
  // There are lots of places we can change the final volume level.
  // For now lets set this one once and leave it alone.
  sgtl5000_1.volume(0.25);

  //The line out has a separate level control but it's not meant to be adjusted like the volume function above.
  // If you're not using the line out don't worry about it.
  sgtl5000_1.lineOutLevel(21); //11-32, the smaller the louder. 21 is about 2 Volts peak to peak

  waveform1.begin(.5, 10.0, WAVEFORM_SAMPLE_HOLD); //(amplitude, frequency, shape)
  waveform1.offset(.5); //DC offset, moves the waveform up and down
  waveform1.phase(0); //time offset, sets when it starts

  waveform2.begin(.5, 1.1, WAVEFORM_SINE);
  waveform2.offset(.5);

  waveform3.begin(1, 0, WAVEFORM_SINE);
  waveform4.begin(1, 0, WAVEFORM_SINE);

  mixer1.gain(0, .5);
  mixer1.gain(1, 0);
  mixer1.gain(3, .3); //feedback

  mixer2.gain(0, .5); //dry waveform3
  mixer2.gain(1, 0);
  mixer2.gain(2, .5); //wet delay

  delay1.begin(tape_delay_bank, DELAY_SIZE, DELAY_SIZE / 2, 0, 2);

} //setup is over


void loop() {
  current_time = millis();

  float temp1 = analogRead(A14);
  float fb = smooth(1, temp1) / 1023.0; //smooth cant take floats
  mixer1.gain(3, fb); //feedback

  float wetdry = 1.0 - (analogRead(A15) / 1023.0);
  mixer2.gain(0, wetdry); //dry waveform3
  mixer2.gain(2, 1.0 - wetdry); //wet delay

  if (current_time - prev_time[4] > 2) {
    prev_time[4] = current_time;
    dt = map(analogRead(A12), 0, 1023, 0, DELAY_SIZE);
    dt = smooth(2, dt);
    delay1.length(dt);
  }

  // freq[3] = 200.0 + ((int(lfo[1] * 10.0) | int(lfo[2] * 10.0)) * 10.0);
  freq[3] = 200.0 + (lfo[1] * 100.0) + (lfo[2] * 100.0);

  waveform3.frequency(smooth1);

  waveform1.amplitude(analogRead(A10) / 1023.0);
  waveform2.amplitude(analogRead(A11) / 1023.0);


  freq[4] = 200.0 + (lfo[2] * 100.0);
  waveform4.frequency(freq[4]);

  for (int j = 0; j < NUM_BUTTONS; j++)  {
    buttons[j].update();
  }

  if ( buttons[0].fell() ) {
  }

  if ( buttons[0].rose() ) {
  }

  if (buttons[0].read() == 0) {
    //do something if the button on the left is being held down
  }

  if (current_time - prev_time[3] > 2) {
    prev_time[3] = current_time;
    pcell1 = analogRead(A8);
    smooth1 = smooth(0, pcell1); //(select, input) select should be a differnt number for every diffent varible yo uwant to smooth.
  }

  if (current_time - prev_time[2] > 50) {
    prev_time[2] = current_time;
    //Serial.print(fb);
    // Serial.print(" ");
    Serial.println(dt);
  }



  if (peak1.available()) {
    lfo[1] = peak1.read();
  }

  if (peak2.available()) {
    lfo[2] = peak2.read();
  }

  if (current_time - prev_time[1] > 33) { //33 milliseconds is about 30 Hz, aka 30 fps
    prev_time[1] = current_time;

    rainbow += 0.0025;
    if (rainbow > 1.0) {
      rainbow -= 1.0;
    }

    //there's another function in this sketch bellow the loop which makes it easier to control the LEDs more info bellow the loop
    //(led to change, hue,saturation,brightness)
    set_LED(0, 0, 0, lfo[1]);
    set_LED(1, rainbow, 1, lfo[2]);
    LEDs.show(); //send these values to the LEDs
  }

  if (current_time - prev_time[0] > 500 && 0) { //change to && 0 to not do this code
    prev_time[0] = current_time;

    //Here we print out the usage of the audio library
    // If we go over 90% processor usage or get near the value of memory blocks we set aside in the setup we'll have issues or crash.
    // If you're using too many block, jut increase the number up top until you're over it by a couple
    Serial.print("processor: ");
    Serial.print(AudioProcessorUsageMax());
    Serial.print("%    Memory: ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();
  }

}// loop is over


////////////////LED function

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

  if (fs > 1.0) {
    fs = 1.0;
  }

  if (fv > 1.0) {
    fv = 1.0;
  }

  //wrap the hue around but if it's over 100 or under -100 cap it at that
  if (fh > 100) {
    fh = 100;
  }

  if (fh < -100) {
    fh = -100;
  }
  //keep subtracting or adding 1 untill it's in the range of 0-1.0
  while (fh > 1.0) {
    fh -= 1.0;
  }
  while (fh < 0) {
    fh += 1.0;
  }

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


////////////smooth function
//based on https://playground.arduino.cc/Main/DigitalSmooth/

#define filterSamples   17   // filterSamples should  be an odd number, no smaller than 3. Incerease for more smoooothness
#define array_num 8 //numer of differnt smooths we can take, one for each pot
int sensSmoothArray[array_num] [filterSamples];   // array for holding raw sensor values for sensor1

int smooth(int array_sel, int input) {
  int j, k, temp, top, bottom;
  long total;
  static int i;
  static int sorted[filterSamples];
  boolean done;

  i = (i + 1) % filterSamples;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable
  sensSmoothArray[array_sel][i] = input;                 // input new data into the oldest slot

  // Serial.print("raw = ");

  for (j = 0; j < filterSamples; j++) { // transfer data array into anther array for sorting and averaging
    sorted[j] = sensSmoothArray[array_sel][j];
  }

  done = 0;                // flag to know when we're done sorting
  while (done != 1) {      // simple swap sort, sorts numbers from lowest to highest
    done = 1;
    for (j = 0; j < (filterSamples - 1); j++) {
      if (sorted[j] > sorted[j + 1]) {    // numbers are out of order - swap
        temp = sorted[j + 1];
        sorted [j + 1] =  sorted[j] ;
        sorted [j] = temp;
        done = 0;
      }
    }
  }

  // throw out top and bottom 15% of samples - limit to throw out at least one from top and bottom
  bottom = max(((filterSamples * 15)  / 100), 1);
  top = min((((filterSamples * 85) / 100) + 1  ), (filterSamples - 1));   // the + 1 is to make up for asymmetry caused by integer rounding
  k = 0;
  total = 0;
  for ( j = bottom; j < top; j++) {
    total += sorted[j];  // total remaining indices
    k++;
  }

  return total / k;    // divide by number of samples
}
