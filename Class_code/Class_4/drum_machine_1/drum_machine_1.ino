/*
  Use arrays to play a beat with the drum and noise objects
*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthSimpleDrum     drum1;          //xy=151,187
AudioSynthSimpleDrum     drum2;          //xy=166,235
AudioSynthNoiseWhite     noise1;         //xy=170,294
AudioEffectEnvelope      envelope1;      //xy=340,291
AudioMixer4              mixer1;         //xy=443,217
AudioOutputI2S           i2s1;           //xy=619,210
AudioConnection          patchCord1(drum1, 0, mixer1, 0);
AudioConnection          patchCord2(drum2, 0, mixer1, 1);
AudioConnection          patchCord3(noise1, envelope1);
AudioConnection          patchCord4(envelope1, 0, mixer1, 2);
AudioConnection          patchCord5(mixer1, 0, i2s1, 0);
AudioConnection          patchCord6(mixer1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=408,113
// GUItool: end automatically generated code


#include <Bounce2.h>
#define NUM_BUTTONS 8
const int BUTTON_PINS[NUM_BUTTONS] = {30, 31, 32, 33, 34, 35, 36, 37};
Bounce * buttons = new Bounce[NUM_BUTTONS];
#define BOUNCE_LOCK_OUT

//then you can declare any variables you want.
unsigned long current_time;
unsigned long prev_time[8]; //an array of 8 variables all named "prev_time"

//starts at midi note 12, C0
PROGMEM const static float chromatic[121] = {16.3516, 17.32391673, 18.35405043, 19.44543906, 20.60172504, 21.82676736, 23.12465449, 24.499718, 25.95654704, 27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192, 8869.845359, 9397.273811, 9956.06479, 10548.08321, 11175.30488, 11839.82309, 12543.8556, 13289.75207, 14080.00185, 14917.24233, 15804.26772, 16744.03838};

//Three, sixteen step sequencers
int seq[3][16] = {
  {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1},
  {1, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1},
  {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0},
};

//the first loop there might be some odd things happening due to some bust w the 4.1 and audio library

int melody_location;
unsigned long noise_timer;
int noise_latch;
int melody_speed;

void setup() {
  //there's a lot we need to do in setup now but most of it is just copy paste.
  // This first group should only be done in setup how much RAM to set aside for the audio library to use.
  // The audio library uses blocks of a set size so this is not a percentage or kilobytes, just a kind of arbitrary number.
  // On our Teensy 4.1 we can go up to almost 2000 but that won't leave any RAM for anyone else.
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


  mixer1.gain(0, .5); //kick
  mixer1.gain(1, .3); //pew
  mixer1.gain(2, .2); //noise

  drum1.frequency(100); //starting freq
  drum1.length(75);//how long to fade out
  drum1.pitchMod(.55); //less than .5 the pitch will rise, great and it will drop

  drum2.frequency(880); //starting freq
  drum2.length(200);//how long to fade out
  drum2.pitchMod(.75); //less than .5 the pitch will rise, great and it will drop

  envelope1.attack(1);
  envelope1.decay(0);
  envelope1.sustain(1);
  envelope1.release(4); 

  noise1.amplitude(1);


  analogReadAveraging(64); //take lots of readings and average them each time we do analogRead. This is one stage of smoothing

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
    buttons[i].interval(10);              // interval in ms
  }

} //setup is over


void loop() {
  current_time = millis();

  //melody_speed = 1000;
  melody_speed = (analogRead(A10) / 1023.0) * 500; //0-500
  if (current_time - prev_time[2] > melody_speed) { //step through the sequence which hold our melody
    prev_time[2] = current_time;

    melody_location++; //same as saying melody_location = melody_location + 1
    if (melody_location > 15) {
      melody_location = 0;
    }

    int temp0 = seq[0][melody_location];
    if (temp0 > 0) {
      drum1.noteOn(); //Unlike the envelope a drum will just play for a certain amount of time
    }

    int temp1 = seq[1][melody_location];
    if (temp1 > 0) {
      envelope1.noteOn();
      noise_timer = current_time; //remember the time so we can tune the envelope off at a set time
      noise_latch = 1;
    }

    int temp2 = seq[2][melody_location];
    if (temp2 > 0) {
      int r1 = random(24);
      drum2.frequency(chromatic[60 + r1]); //use chromatic for the frequency
      drum2.noteOn(); //Unlike the envelope a drum will just play for a certain amount of time
    }
  }

  if (current_time - noise_timer > 5 && noise_latch == 1) {
    envelope1.noteOff();
    noise_latch = 0; //so it won't keep happening every loop
  }


  if (current_time - prev_time[0] > 500 && 1) { //change "&& 0" to "&& 1" to print this
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
