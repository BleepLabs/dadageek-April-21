/*
  This code shows three types of sampling
  - Copy from the SD card ti internal RAM using SD2RAM
  - Record from the input into RAM
  - Play from program memory using WAV2SKETCH

  more info on sampling and memory types here 
  https://github.com/BleepLabs/dadageek-April-21/wiki/Memory-and-sampling

  The samplers really just play back or record to sections of memory. 
  The have one input for sampling sample. Right now they are not stereo but i'll fix this soonish
  You can have multiple ones lookign at the same part of memory

  Since "AudioSampler" is not in the regular library use "AudioEffectChorus" as it has the same ins and outs
  Then just replace "AudioEffectChorus" with "MemSampler" and "chorusX" with "samperX"
  
  Sampler functions:
  begin(array select, array length); //do this in setup
  start_location(int location in samples); //Set the start at any postion in the sample
  play_length(int length in samples); //how long to play
  play(); //start the sample playing at the selected start location and length. It will play the entire length unless stopped
  stop(); //stop the sample playing or recording
  frequency(float speed ratio); //1.0 plays it back at the rate is was recorded. .5 is half speed, 2 is 2x speed
  loop(0 or 1); //1 to loop this sampler
  reverse(0 or 1); //1 to play it backwards
*/

#include "mem_sampler.h" //include beore the rest

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
MemSampler          sampler2;       //xy=118,397
MemSampler          sampler1;       //xy=123,348
MemSampler          sampler3;       //xy=146,441
MemSampler          sampler4;       //xy=177,490
AudioInputI2S            i2s1;           //xy=176,256
AudioAmplifier           amp1;           //xy=364,232
AudioMixer4              mixer1;         //xy=421,310
AudioRecordQueue         queue1;         //xy=561,166
AudioMixer4              mixer2;         //xy=600,296
AudioOutputI2S           i2s2;           //xy=789,311
AudioConnection          patchCord1(sampler2, 0, mixer1, 1);
AudioConnection          patchCord2(sampler1, 0, mixer1, 0);
AudioConnection          patchCord3(sampler3, 0, mixer1, 2);
AudioConnection          patchCord4(i2s1, 0, amp1, 0);
AudioConnection          patchCord5(sampler4, 0, mixer1, 3);
AudioConnection          patchCord6(amp1, 0, sampler2, 0);
AudioConnection          patchCord7(amp1, 0, mixer2, 0);
AudioConnection          patchCord8(mixer1, 0, mixer2, 1);
AudioConnection          patchCord9(mixer2, 0, i2s2, 0);
AudioConnection          patchCord10(mixer2, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=507,419
// GUItool: end automatically generated code


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

//this reads the buttons and can be left alone
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
unsigned int sd_wav_sample_len;
unsigned int input_sample_len;

float sampler_freq[4];
float amp[4];

//make this the size of the file you want to copy in integers. More info in setup
// must be "uint16_t"
uint16_t SDbank0[45000]; 

#define input_bank_max_length 44100  //1 second of space max. 
// must be "uint16_t"
uint16_t inputSampleBank0[input_bank_max_length];

#include "samples.h"
#include "extras.h" //include these right before setup


void setup() {

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(BUILTIN_SDCARD)) {
    delay(500);
    Serial.println("Card failed or not present");
    delay(2000);
  }
  Serial.println("card initialized.");

  //SD2RAM copies the file in quatoations from the root of the SD card to the bank specified
  // Make sure the bank is big enough to fit it. On your computer simply see how may bytes it is and make the bank that big in the initilaization sections
  // it reurns the acutal size of the audio so the sample will end at just the right time
  // this is currently jsut for mono wav files that are 16bit but I'll be adding more features soon
  //sd_wav_sample_len = SD2RAM("X.wav", SDbank0); //file name inside " " , bank to put it in

  LEDs.begin(); //must be done in setup for the addressable LEDs to work.
  //here is a basic way of writing to the LEDs.
  LEDs.setPixelColor(0, 0, 0, 0); //(LED number, red level, green level, blue level). All levels are 0-255
  LEDs.setPixelColor(1, 0, 0, 0);
  LEDs.show(); //send these values to the LEDs

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  ); //setup the bounce instance for the current button
    buttons[i].interval(10);  // interval in milliseconds. How long after the first change will ignore noise
  }

  //what we put in RAM, as in the arrays for the audio in or SD card samples, 
  // is differnt that setting space aside for audio
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

  //The line out has a seperate level control but it's not meant to be adjusted like the volume function above.
  // If you're not using the line out don't worry about it.
  sgtl5000_1.lineOutLevel(21); //11-32, the smaller the louder. 21 is about 2 Volts peak to peak

  //samples are not 100% volume all the time so these don't need to add up to 1
  mixer1.gain(0, .25); //sampler1
  mixer1.gain(1, .25); //sampler2
  mixer1.gain(2, .5); //sampler3
  mixer1.gain(3, .25); //sampler4

  mixer2.gain(0, 0); //audio input
  mixer2.gain(1, 1); //from mixer1

  amp1.gain(1); //used to attenuate or amplify the incoming siganl before it goes to the sampler input

  //(bank name, length of sample)
  //length of sample is returned by SDtoRAM. It's a little differnt than the actual file length
  sampler1.begin(SDbank0, sd_wav_sample_len);
  sampler1.loop(0); //1 too loop
  sampler1.frequency(1.0); //1.0 is normal speed. .5 would be half as slow, 2 would be twice as fast
  sampler1.reverse(0); //1 to pay in reverse

  //for use with the audio input sampler
  sampler2.begin(inputSampleBank0, input_bank_max_length);
  sampler2.loop(0);
  sampler2.frequency(1.0);
  sampler2.reverse(0);

  //you can also convert samples to arrays and store them in program memory aka flash
  // these samples live in samples.h
  // info here https://github.com/BleepLabs/dadageek-April-21/wiki/Memory-and-sampling#wav2sketch
  sampler3.begin(clap626,clap626_length);
  sampler3.loop(0);
  sampler3.frequency(1.0);
  sampler3.reverse(0);

  sampler4.begin(dmx_kick,dmx_kick_length);
  sampler4.loop(0);
  sampler4.frequency(1.0);
  sampler4.reverse(0);
  
} //setup is over


void loop() {
  current_time = millis();

  for (int j = 0; j < NUM_BUTTONS; j++)  {
    buttons[j].update();
  }

  if ( buttons[0].fell() ) {
    //sampler1.start_location(sd_wav_sample_len / 2); //where to start playing. Measured in samples
    //sampler1.play_length(5000); //how long to play. Measured in samples
    sampler1.play();
  }

  if ( buttons[0].rose() ) {
    sampler1.stop(); //only play while the button is being held down
  }

  if ( buttons[1].fell() ) {
    sampler2.play();
  }

  if ( buttons[2].fell() ) {
    sampler3.play();
  }

  if ( buttons[3].fell() ) {
    sampler4.play();
  }

  if ( buttons[7].fell() ) {
    sampler2.record();
  }

  if ( buttons[7].read() == 0) {
  }

  if ( buttons[7].rose() ) {
    sampler2.stop();

  }


  if (current_time - prev_time[2] > 5) { //slow all this down a little to reduce jiggly readings
    prev_time[2] = current_time;
    
    mixer2.gain(0, smooth(1, analogRead(A10)) / 1023.0); //audio input

    sampler_freq[0] = (smooth(0, analogRead(A14)) / 1023.0) * 3.0;
    sampler1.frequency(sampler_freq[0]);
    sampler2.frequency(sampler_freq[0]);
    sampler3.frequency(sampler_freq[0]);
    sampler4.frequency(sampler_freq[0]);

  }


  if (current_time - prev_time[0] > 500 && 1) {
    prev_time[0] = current_time;

    Serial.print(AudioProcessorUsageMax());
    Serial.print("% ");
    Serial.print(AudioMemoryUsageMax());
    Serial.println();
    AudioProcessorUsageMaxReset(); //We need to reset these values so we get a real idea of what the audio code is doing rather than just peaking in every half a second
    AudioMemoryUsageMaxReset();
  }

}// loop is over
