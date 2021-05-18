/*
  Based on this device
  https://modwiggler.com/forum/viewtopic.php?t=179501
  https://minigene.myshopify.com/

  each pot controls the sound of a step
  tap button 30 to control speed

*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=296,416
AudioSynthNoiseWhite     noise1;         //xy=300,383
AudioSynthNoiseWhite     noise2;         //xy=307,447
AudioSynthNoiseWhite     noise3;         //xy=368,527
AudioSynthSimpleDrum     drum2;          //xy=414,584
AudioSynthSimpleDrum     drum3;          //xy=415,621
AudioEffectEnvelope      envelope2;      //xy=444,416
AudioEffectEnvelope      envelope1;      //xy=446,383
AudioEffectEnvelope      envelope3;      //xy=447,447
AudioSynthSimpleDrum     drum1;          //xy=513,489
AudioEffectEnvelope      envelope4;      //xy=526,529
AudioMixer4              mixer3;         //xy=577,589
AudioMixer4              mixer1;         //xy=599,423
AudioMixer4              mixer2;         //xy=741,510
AudioOutputI2S           i2s1;           //xy=956,483
AudioConnection          patchCord1(waveform1, envelope2);
AudioConnection          patchCord2(noise1, envelope1);
AudioConnection          patchCord3(noise2, envelope3);
AudioConnection          patchCord4(noise3, envelope4);
AudioConnection          patchCord5(drum2, 0, mixer3, 0);
AudioConnection          patchCord6(drum3, 0, mixer3, 1);
AudioConnection          patchCord7(envelope2, 0, mixer1, 1);
AudioConnection          patchCord8(envelope1, 0, mixer1, 0);
AudioConnection          patchCord9(envelope3, 0, mixer1, 2);
AudioConnection          patchCord10(drum1, 0, mixer2, 1);
AudioConnection          patchCord11(envelope4, 0, mixer2, 2);
AudioConnection          patchCord12(mixer3, 0, mixer2, 3);
AudioConnection          patchCord13(mixer1, 0, mixer2, 0);
AudioConnection          patchCord14(mixer2, 0, i2s1, 0);
AudioControlSGTL5000     sgtl5000_1;     //xy=760,332
// GUItool: end automatically generated code


#define num_of_leds 20 //increase for external LEDs
float max_brightness = .1; //change this to increase the max brightness of the LEDs. 1.0 is crazy bright
//everything else can be left alone

#include <WS2812Serial.h> //include the code from this file in our sketch  https://github.com/PaulStoffregen/WS2812Serial/archive/refs/heads/master.zip
#define led_data_pin 29
byte drawingMemory[num_of_leds * 3];
DMAMEM byte displayMemory[num_of_leds * 12];
WS2812Serial LEDs(num_of_leds, displayMemory, drawingMemory, led_data_pin, WS2812_GRB);



//this can be left alone
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
float freq1;
int seq_location;
int seq[8];
int tempo_led_flag;
unsigned long led_timer;
float tempo_led_bright;
int clock_div;
unsigned long tap_reads[3];
int tap_read_sel;
int tap_read_average = 120;
unsigned long last_tap;
unsigned long current_tap;

void setup() {
  LEDs.begin(); //must be done in setup for the addressable LEDs to work.

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


  noise1.amplitude(.5);                      //snare body noise
  noise2.amplitude(1);                       //snare hit noise
  waveform1.begin (1, 220, WAVEFORM_SINE);   //snare body wave

  noise3.amplitude(.5);                      //hi hat thingy

  envelope1.attack(10);      //snare body noise
  envelope1.decay( 200);
  envelope1.sustain(0);
  envelope1.release(100);

  envelope2.attack(10);      //snare body wave
  envelope2.decay( 200);
  envelope2.sustain(0);
  envelope2.release(100);

  envelope3.attack(0);     //snare hit noise
  envelope3.decay( 20);
  envelope3.sustain(0);
  envelope3.release(20);

  envelope4.attack(0);     //hi hat thingy
  envelope4.decay( 10);
  envelope4.sustain(0);
  envelope4.release(10);

  drum1.frequency(101);    //kick
  drum1.length(180);
  drum1.pitchMod(0.5);

  mixer1.gain(0, .2);    //noise1   gain(channel,level)
  mixer1.gain(1, .4);    //waveform1
  mixer1.gain(2, .6);    //noise2

  mixer2.gain(0, .4);   //snare
  mixer2.gain(1, .6);   //drum1 aka kick
  mixer2.gain(2, .2);   //closed hat??
  mixer2.gain(3, .5);   //toms

  drum2.frequency(440);     //tom
  drum2.length(180);
  drum2.pitchMod(0.5);

  drum3.frequency(220);    //tom two
  drum3.length(180);
  drum3.pitchMod(0.5);

  mixer3.gain(0, .5);     //tom mixer
  mixer3.gain(0, .5);


  //begin the waveforms and setup the mixer leves here


} //setup is over

void loop() {
  current_time = millis();

  seq[0] = (1023 - analogRead(A10));
  seq[1] = (1023 - analogRead(A11));
  seq[2] = (1023 - analogRead(A12));
  seq[3] = (1023 - analogRead(A13));
  seq[4] = (1023 - analogRead(A14));
  seq[5] = (1023 - analogRead(A15));
  seq[6] = (1023 - analogRead(A16));
  seq[7] = (1023 - analogRead(A17));

  if (current_time - prev_time[1] > tap_read_average) {   //last figure (120 sets tempo.
    prev_time[1] = current_time;

    clock_div++;
    if (clock_div > 3) {
      clock_div = 0;
      tempo_led_flag = 1;
      led_timer = current_time;
      tempo_led_bright = 1;
    }

    seq_location++;
    if (seq_location > 7) {
      seq_location = 0;
    }

    int temp1 = seq[seq_location];

    if (temp1 >= 104 && temp1 < 287) {
      drum1.noteOn();
    }

    if (temp1 >= 288 && temp1 < 471) {
      envelope1.noteOn();
      envelope2.noteOn();
      envelope3.noteOn();
    }
    if (temp1 >= 472 && temp1 < 655) {
      envelope4.noteOn();
    }
    if (temp1 >= 656 && temp1 < 839) {
      drum2.noteOn();
    }
    if (temp1 >= 840 && temp1 < 1023) {
      drum3.noteOn();
    }

  }

  if (current_time - led_timer > 10 && tempo_led_flag == 1) {
    set_LED(0, .5, .5, tempo_led_bright);
    tempo_led_bright = tempo_led_bright * .9;
    LEDs.show();
  }



  for (int j = 0; j < NUM_BUTTONS; j++)  {
    buttons[j].update();
  }

  if ( buttons[0].fell() ) {
    tap_read_sel++;
    if (tap_read_sel > 2) {
      tap_read_sel = 0;
    }
    last_tap = current_tap;
    current_tap = current_time;

    tap_reads[tap_read_sel] = current_tap - last_tap;

    for (int j = 0; j < 3; j++) {
      tap_read_average = tap_read_average + tap_reads[j];
    }
    tap_read_average = (tap_read_average / 3) / 4;
    Serial.println(tap_read_average);

  }



  if (current_time - prev_time[0] > 500 && 0) { //cahnge to && 0 to not do this code
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



} // loop is over


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
