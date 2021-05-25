#include "effect_tape_delay.h" // this needs to be before the other audio code 

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform1;      //xy=221.61905670166016,624.7936458587646
AudioEffectEnvelope      envelope1;      //xy=250.74617385864258,682.9683494567871
AudioEffectFlange        flange1;        //xy=394.4444332122803,758.8888778686523
AudioEffectChorus        chorus1;        //xy=406.6667022705078,713.333402633667
AudioAmplifier           amp1;           //xy=448.7301025390625,838.0158653259277
AudioEffectTapeDelay         delay1;         //xy=455.349178314209,1093.6507987976074
AudioMixer4              mixer1;         //xy=461.6031684875488,930.2380752563477
AudioMixer4              mixer3;         //xy=584.4444618225098,685.5555629730225
AudioMixer4              mixer2;         //xy=616.3016815185547,832.6667404174805
AudioOutputI2S           i2s1;           //xy=881.6032180786133,668.5714731216431
AudioConnection          patchCord1(waveform1, envelope1);
AudioConnection          patchCord2(envelope1, chorus1);
AudioConnection          patchCord3(envelope1, 0, mixer3, 0);
AudioConnection          patchCord4(envelope1, flange1);
AudioConnection          patchCord5(flange1, 0, mixer3, 2);
AudioConnection          patchCord6(chorus1, 0, mixer3, 1);
AudioConnection          patchCord7(amp1, 0, mixer1, 0);
AudioConnection          patchCord8(amp1, 0, mixer2, 0);
AudioConnection          patchCord9(delay1, 0, mixer2, 1);
AudioConnection          patchCord10(delay1, 0, mixer1, 1);
AudioConnection          patchCord11(mixer1, delay1);
AudioConnection          patchCord12(mixer3, amp1);
AudioConnection          patchCord13(mixer2, 0, i2s1, 0);
AudioConnection          patchCord14(mixer2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=782.15869140625,1003.079384803772
// GUItool: end automatically generated code

#define FLANGER_DELAY_LENGTH (6*AUDIO_BLOCK_SAMPLES)
short flanger_bank[FLANGER_DELAY_LENGTH];

#define CHORUS_DELAY_LENGTH (40*AUDIO_BLOCK_SAMPLES)
short chorus_bank[CHORUS_DELAY_LENGTH];

#define DELAY_SIZE 20000 //size in 2xintegers
int16_t tape_delay_bank[DELAY_SIZE]; //int16_t is a more specific way of saying integer
float input_volume, feedback_level, wet_level, dry_level;
int delay_time_adjust;

//#include "bleep_base.h" //Then we can add this line that we will still need

#include <Bounce2.h>
#define NUM_BUTTONS 8
const int BUTTON_PINS[NUM_BUTTONS] = {30, 31, 32, 33, 34, 35, 36, 37};
Bounce * buttons = new Bounce[NUM_BUTTONS];
#define BOUNCE_LOCK_OUT

unsigned long current_time;
unsigned long prev_time[8]; //an array of 8 variables all named "prev_time"
int new_note_flag;
int new_note_number;
float pw1;
int pw_flag;
//starts at midi note 12, C0 https://newt.phys.unsw.edu.au/jw/notes.html
PROGMEM const static float chromatic[121] = {16.3516, 17.32391673, 18.35405043, 19.44543906, 20.60172504, 21.82676736, 23.12465449, 24.499718, 25.95654704, 27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192, 8869.845359, 9397.273811, 9956.06479, 10548.08321, 11175.30488, 11839.82309, 12543.8556, 13289.75207, 14080.00185, 14917.24233, 15804.26772, 16744.03838};


void setup() {
  AudioMemory(1200);
  sgtl5000_1.enable(); //Turn the adapter board on
  sgtl5000_1.lineOutLevel(25); //11-32, the smaller the louder. 21 is about 2 Volts peak to peak

  waveform1.begin(0.5, 420.0, WAVEFORM_BANDLIMIT_PULSE);

  delay1.begin(tape_delay_bank, DELAY_SIZE, DELAY_SIZE / 2, 0, 2);

  chorus1.begin(chorus_bank, CHORUS_DELAY_LENGTH, 3);
  //begin(delayBuffer, length, offset, depth, delayRate);

  flange1.begin(flanger_bank, FLANGER_DELAY_LENGTH-1, FLANGER_DELAY_LENGTH / 2, FLANGER_DELAY_LENGTH / 4, .25);

  envelope1.attack(40);
  envelope1.decay(250);
  envelope1.sustain(0.5);
  envelope1.release(500);

  // INTERFACE
  analogReadAveraging(64);
  // BUTTONS
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
    buttons[i].interval(10);              // interval in ms
  }
}

void loop() {
  current_time = millis();

  // read knobs
  if (current_time - prev_time[3] > 3) {
    prev_time[3] = current_time;

    input_volume = 1.0 - (analogRead(A10) / 1023.0); //we can amplify or attenuate the signal
    amp1.gain(input_volume);

    feedback_level = 1.0 - (analogRead(A11) / 1023.0);
    mixer1.gain(1, feedback_level); //how much of the delay output comes back into the input mixer

    dry_level = smooth(1, analogRead(A12)) / 1023.0;
    wet_level = 1.0 - dry_level; //as one goes up the other goes down to control the wet dry mix.
    mixer2.gain(0, dry_level); // signal straigh from input
    mixer2.gain(1, wet_level); //signal from delay output

    delay_time_adjust = (analogRead(A13) / 1023.0) * DELAY_SIZE; //delay size is the max so jsut multiply it bu the 0-1.0 pot
    delay_time_adjust = smooth(0, delay_time_adjust);
    delay1.length(delay_time_adjust);

    // float pw1 = smooth(3, analogRead(A14)) / 1023.0;
    //waveform1.pulseWidth(pw1);

    float wd2 = (analogRead(A15) / 1023.0);
    mixer3.gain(0, 0);
    //mixer3.gain(1, 1.0 - wd2);
    mixer3.gain(2, .5);

    if (pw_flag == 1) {
      pw1 += .005;
      if (pw1 > .9) {
        pw1 = .9;
        pw_flag = 0;
      }
    }
  }

  // read buttons
  for (int j = 0; j < NUM_BUTTONS; j++)  {
    buttons[j].update();
    if ( buttons[j].fell() ) {
      new_note_flag = 1;
      new_note_number = j;
      pw1 = 0;
      pw_flag = 1;
      Serial.print(j);
      Serial.println();
    }

    if ( buttons[j].rose() ) {
      new_note_flag = 0;
    }
  }

  if (new_note_flag == 1) {
    envelope1.noteOn();
    //waveform1.begin(0.25, 420.69 * (1 + new_note_number * 0.14141), WAVEFORM_SQUARE);
    waveform1.frequency(chromatic[40 + new_note_number]);
    new_note_flag = 2;

  }

  if (new_note_flag == 0) {
    envelope1.noteOff();
    new_note_flag = 2;
  }



  if (current_time - prev_time[2] > 50 && 0) { //change "&& 0" to "&& 1" to print this
    prev_time[2] = current_time;
    //    Serial.print("feedback_level");
    //    Serial.println();
    //    Serial.print(analogRead(A11));
    //    Serial.println();
    //    Serial.print(feedback_level);
    Serial.println(pw1);
  }

  // debug viewer

  if (current_time - prev_time[0] > 500 && 1 ) { //change "&& 0" to "&& 1" to print this
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
