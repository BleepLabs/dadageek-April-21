/*

  Record the position of a pot and play it back


  https://bleeplabs.github.io/AudioGUI-Bleep/?info=MemSampler

  This code shows three types of sampling
  - Copy from the SD card to internal RAM using SD2RAM
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
  record(); starts recoding the input to the array
*/



#include "mem_sampler.h" //include beore the rest

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=190,139
MemSampler               sampler4;       //xy=208,483
MemSampler               sampler3;       //xy=253,429
MemSampler               sampler2;       //xy=279,366
MemSampler               sampler1;       //xy=366,322
AudioEffectGranular      granular1;      //xy=384,626
AudioAmplifier           amp1;           //xy=403,196
AudioMixer4              mixer1;         //xy=509,416
AudioAnalyzePeak         peak1;          //xy=596,298
AudioRecordQueue         queue1;         //xy=658,196
AudioMixer4              mixer2;         //xy=753,301
AudioOutputI2S           i2s2;           //xy=908,309
AudioConnection          patchCord1(i2s1, 0, amp1, 0);
AudioConnection          patchCord2(sampler4, 0, mixer1, 3);
AudioConnection          patchCord3(sampler3, 0, mixer1, 2);
AudioConnection          patchCord4(sampler2, 0, mixer1, 1);
AudioConnection          patchCord5(sampler1, 0, mixer1, 0);
AudioConnection          patchCord6(sampler1, peak1);
AudioConnection          patchCord7(amp1, sampler2);
AudioConnection          patchCord8(amp1, 0, mixer2, 0);
AudioConnection          patchCord9(mixer1, 0, mixer2, 1);
AudioConnection          patchCord10(mixer2, 0, i2s2, 0);
AudioConnection          patchCord11(mixer2, 0, i2s2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=582,574
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
uint16_t SDbank0[1000];

//needs to be double the length you need to store so this will be one second
#define input_bank_max_length 88100
// must be "uint16_t"
//uint16_t inputSampleBank0[input_bank_max_length];

float chop_ratio_step1;
int chop_ratio_step2;
float peak1_read, prev_peak1_read;

long chops[100];
int chop_loc;
int chop_flag;
unsigned long chop_time;

#define auto_max_length 5000
byte automation[auto_max_length]; //save space by making it a byte. an int is actually 4 bytes
int auto_play_index, auto_rec_index;
int pot_in, pot_out;


#include "samples.h"
#include "extras.h" //include these right before setup


void setup() {

  //turn off audio code
  // this is necessary to avoid glitches while loading samples
  AudioNoInterrupts ();


  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(BUILTIN_SDCARD)) {
    delay(500);
    Serial.println("Card failed or not present");
    delay(2000);
  }
  Serial.println("card initialized.");

  //SD2RAM copies the file in quoations from the root of the SD card to the bank specified
  // Make sure the bank is big enough to fit it. On your computer simply see how may bytes it is and make the bank that big in the initilaization sections
  // it reurns the acutal size of the audio so the sample will end at just the right time
  // this is currently jsut for mono wav files that are 16bit but I'll be adding more features soon

  // sd_wav_sample_len = SD2RAM("taiko2.wav", SDbank0); //file name inside " " , bank to put it in

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
  sampler1.begin(break1, break1_length);
  sampler1.loop(1); //1 too loop
  sampler1.frequency(1.0); //1.0 is normal speed. .5 would be half as slow, 2 would be twice as fast
  sampler1.reverse(0); //1 to pay in reverse

  //for use with the audio input sampler
  sampler2.begin(break1, break1_length);
  sampler2.loop(0);
  sampler2.frequency(1.0);
  sampler2.reverse(0);

  //you can also convert samples to arrays and store them in program memory aka flash
  // these samples live in samples.h
  // info here https://github.com/BleepLabs/dadageek-April-21/wiki/Memory-and-sampling#wav2sketch
  sampler3.begin(break1, break1_length);
  sampler3.loop(0);
  sampler3.frequency(1.0);
  sampler3.reverse(0);

  sampler4.begin(break1, break1_length);
  sampler4.loop(0);
  sampler4.frequency(1.0);
  sampler4.reverse(0);


  AudioInterrupts();  //turn audio code bank on


} //setup is over


void loop() {
  current_time = millis();

  for (int j = 0; j < NUM_BUTTONS; j++)  {
    buttons[j].update();
  }

  if ( buttons[0].fell() ) {
    sampler1.play();
    chop_loc = 0; //restart the chop counter
    chop_flag = 0;
    peak1_read = 0; //other wise prev_peak1_read is still the last reading it took
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

  if ( buttons[6].fell() ) {
    auto_rec_index = 0;
  }

  if ( buttons[5].fell() ) {
    auto_play_index = 0;
  }

  if ( buttons[6].read() == 0) { //record the pot
    if (current_time - prev_time[4] > 2) { //500Hz sampling rate. You can probably go down to 100Hz and still be smooth enough
      prev_time[4] = current_time;

      auto_rec_index++;
      if (auto_rec_index > auto_max_length) { //jsut kees looping, doesnt reset or turn off
        auto_rec_index = 0;
      }
      pot_in = analogRead(A17);
      automation[auto_rec_index] = pot_in / 4; //analog read is 10 bits but the array is a byte, 8 bites  1024/4=256. YOu can bee a cool nerd and do >>2, bitshift right by 2
    }
  }

  if ( buttons[5].read() == 0) { //play back
    if (current_time - prev_time[5] > 2) { //same rate but of course you could vary it
      prev_time[5] = current_time;
      auto_play_index++;
      if (auto_play_index > auto_max_length) {
        auto_play_index = 0;
      }
      pot_out = automation[auto_play_index] ;
      Serial.println(pot_out);

    }
  }


  if (peak1.available()) {

    prev_peak1_read = peak1_read;
    peak1_read = peak1.read();

    //for some reason it gets the first chop the first time you run it but dones't do it again hmmmm
    // works pretty well though

    //float target = analogRead(A16) / 1023.0;
    float target = .4; //found by printing out peak
    if (prev_peak1_read < target && peak1_read > target) {
      if (chop_flag == 0) { //we want to wait a bit before recording another chop location
        chop_time = current_time;
        chop_flag = 1;
        chops[chop_loc] = sampler1.current_location();
        Serial.println(chops[chop_loc]);
        chop_loc++;
      }
    }
  }


  if (current_time - chop_time > 100) {
    chop_flag = 0;
  }

  if (current_time - prev_time[2] > 5) { //slow all this down a little to reduce jiggly readings
    prev_time[2] = current_time;

    mixer2.gain(0, smooth(1, analogRead(A10)) / 1023.0); //audio input

    int raw_freq_pot = 1023 - smooth(0, analogRead(A14));

    if (raw_freq_pot < 512) {
      float temp1 = 4.0 - ((raw_freq_pot / 512.0) * 4.0);
      sampler_freq[0] = temp1;
      sampler1.reverse(1);
      sampler2.reverse(1);
      sampler3.reverse(1);
      sampler4.reverse(1);
    }
    else {
      float temp1 = (((raw_freq_pot - 512) / 512.0) * 4.0);
      sampler_freq[0] = temp1;
      sampler1.reverse(0);
      sampler2.reverse(0);
      sampler3.reverse(0);
      sampler4.reverse(0);
    }

    //combine the pot and pot to cahnge the frequency
    //take the pot from 0-255 to -1.0 to 1.0
    sampler_freq[0] = sampler_freq[0] + (((pot_out / 255.0) * 2.0) - 1.0);
    if (sampler_freq[0] < 0) {
      sampler_freq[0] = 0; //dont want to go under 0
    }

    sampler1.frequency(sampler_freq[0]);
    sampler2.frequency(sampler_freq[0]);
    sampler3.frequency(sampler_freq[0]);
    sampler4.frequency(sampler_freq[0]);

    chop_ratio_step1 = (smooth(2, analogRead(A15))) / 1023.0; //0 -1.0
    chop_ratio_step2 = (chop_ratio_step1 * 15) + 1; //1 -16 integers

    int start1 = ((smooth(3, analogRead(A16))) / 1023.0) * nuroto_length;
    float trimmm = .9;

    sampler2.start_location(chops[0]); //where to start playing. Measured in samples
    sampler2.play_length((chops[1] - chops[0])*trimmm); //how long to play. Measured in samples

    sampler3.start_location(chops[1]); //where to start playing. Measured in samples
    sampler3.play_length((chops[2] - chops[1])*trimmm); //how long to play. Measured in samples

    sampler4.start_location(chops[2]); //where to start playing. Measured in samples
    sampler4.play_length((chops[3] - chops[2])*trimmm); //how long to play. Measured in samples

  }


  if (current_time - prev_time[7] > 50 && 0) {
    prev_time[7] = current_time;
    Serial.println(sampler_freq[0]);

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
