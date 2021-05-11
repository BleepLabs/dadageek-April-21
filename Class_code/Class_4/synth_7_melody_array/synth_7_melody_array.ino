/*
  Read the values of oscillators so we can use the to change variables


*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveform       waveform3;      //xy=67,315
AudioSynthWaveform       waveform4;      //xy=77,352
AudioSynthWaveform       waveform2;      //xy=84,275
AudioSynthWaveform       waveform1;      //xy=113,227
AudioSynthWaveform       waveform5;      //xy=136,570
AudioMixer4              mixer1;         //xy=255,310
AudioAnalyzePeak         peak1;          //xy=281,540
AudioEffectEnvelope      envelope1;      //xy=354,215
AudioFilterStateVariable filter1;        //xy=453.9999885559082,287.9999885559082
AudioMixer4              mixer2;         //xy=615,291
AudioEffectDelay         delay1;         //xy=623,441
AudioEffectDelay         delay2;         //xy=810,351
AudioOutputI2S           i2s1;           //xy=895,204
AudioConnection          patchCord1(waveform3, 0, mixer1, 2);
AudioConnection          patchCord2(waveform4, 0, mixer1, 3);
AudioConnection          patchCord3(waveform2, 0, mixer1, 1);
AudioConnection          patchCord4(waveform1, 0, mixer1, 0);
AudioConnection          patchCord5(waveform5, peak1);
AudioConnection          patchCord6(mixer1, envelope1);
AudioConnection          patchCord7(envelope1, 0, filter1, 0);
AudioConnection          patchCord8(filter1, 0, mixer2, 0);
AudioConnection          patchCord9(filter1, 1, mixer2, 1);
AudioConnection          patchCord10(filter1, 2, mixer2, 2);
AudioConnection          patchCord11(mixer2, delay1);
AudioConnection          patchCord12(mixer2, 0, i2s1, 0);
AudioConnection          patchCord13(mixer2, delay2);
AudioConnection          patchCord14(delay1, 0, mixer2, 3);
AudioConnection          patchCord15(delay2, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=210,163
// GUItool: end automatically generated code

#include <Bounce2.h>
#define NUM_BUTTONS 8
const int BUTTON_PINS[NUM_BUTTONS] = {30, 31, 32, 33, 34, 35, 36, 37};
Bounce * buttons = new Bounce[NUM_BUTTONS];
#define BOUNCE_LOCK_OUT

//then you can declare any variables you want.
unsigned long current_time;
unsigned long prev_time[8]; //an array of 8 variables all named "prev_time"
float freq1, spacing1;
int button_reading1;
int prev_button_reading1;
int button_reading2;
int prev_button_reading2;
int button1_pin = 30;
int button2_pin = 31;
int new_note_flag;
int new_note_number;
//starts at midi note 12, C0
PROGMEM const static float chromatic[121] = {16.3516, 17.32391673, 18.35405043, 19.44543906, 20.60172504, 21.82676736, 23.12465449, 24.499718, 25.95654704, 27.50000365, 29.13523896, 30.86771042, 32.7032, 34.64783346, 36.70810085, 38.89087812, 41.20345007, 43.65353471, 46.24930897, 48.99943599, 51.91309407, 55.00000728, 58.27047791, 61.73542083, 65.40639999, 69.29566692, 73.4162017, 77.78175623, 82.40690014, 87.30706942, 92.49861792, 97.99887197, 103.8261881, 110.0000146, 116.5409558, 123.4708417, 130.8128, 138.5913338, 146.8324034, 155.5635124, 164.8138003, 174.6141388, 184.9972358, 195.9977439, 207.6523763, 220.0000291, 233.0819116, 246.9416833, 261.6255999, 277.1826676, 293.6648067, 311.1270248, 329.6276005, 349.2282776, 369.9944716, 391.9954878, 415.3047525, 440.0000581, 466.1638231, 493.8833665, 523.2511997, 554.3653352, 587.3296134, 622.2540496, 659.2552009, 698.4565551, 739.9889431, 783.9909755, 830.6095048, 880.0001162, 932.3276461, 987.7667329, 1046.502399, 1108.73067, 1174.659227, 1244.508099, 1318.510402, 1396.91311, 1479.977886, 1567.981951, 1661.219009, 1760.000232, 1864.655292, 1975.533466, 2093.004798, 2217.46134, 2349.318453, 2489.016198, 2637.020803, 2793.82622, 2959.955772, 3135.963901, 3322.438019, 3520.000464, 3729.310584, 3951.066931, 4186.009596, 4434.92268, 4698.636906, 4978.032395, 5274.041605, 5587.652439, 5919.911543, 6271.927802, 6644.876037, 7040.000927, 7458.621167, 7902.133861, 8372.019192, 8869.845359, 9397.273811, 9956.06479, 10548.08321, 11175.30488, 11839.82309, 12543.8556, 13289.75207, 14080.00185, 14917.24233, 15804.26772, 16744.03838};

int melody1[8] = {32, 33, 35, 0, 44, 50, 47, 0};

float lfo[8] = {30, 0, 100}; //float can contain deciamls
int lfo_latch[8];
int lfo_rate[8];
float lfo_rise_time[3] = {1.1, 1.03, 1.01};
int melody_location, melody_note_number;
unsigned long melody_note_timer;
int melody_speed, gate_length;

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

  //This next group can be done anywhere in the code but we want to start things with these
  // values and change some of them in the loop.

  //Notice we start by writing the object we want, then a period, then the function
  // begin(volume from 0.0-1.0 , frequency , shape of oscillator)
  waveform1.begin(1, 300.0, WAVEFORM_SAWTOOTH);
  waveform2.begin(1, 220.1, WAVEFORM_SAWTOOTH);
  waveform3.begin(1, 220.1, WAVEFORM_SAWTOOTH);
  waveform4.begin(1, 220.1, WAVEFORM_SAWTOOTH);
  waveform5.begin(1, .25, WAVEFORM_SINE);

  //See the tool for more info https://www.pjrc.com/teensy/gui/?info=AudioSynthWaveform
  // but there is also these options which reduce digital aliasing : WAVEFORM_BANDLIMIT_SAWTOOTH, WAVEFORM_BANDLIMIT_SAWTOOTH_REVERSE, WAVEFORM_BANDLIMIT_SQUARE ,WAVEFORM_BANDLIMIT_PULSE

  //Sets the frequency of the filter. We're using lowpass so all frequencies under this can pass unchanged
  // while ones above will be attenuated, the higher up the quieter they will be
  filter1.frequency(15000); //15000 Hz is the top frequency it can do and means pretty much nothing is attenuated
  //resonance aka feedback aka q increases the amplitude at the filter frequency
  filter1.resonance(3.0); //.7 is no change, higher that that makes the classic filter sound


  //delay(output, milliseconds of delay)
  // whatever comes in will come out that output X millis later
  //You can't change this delay time without making lots of noise. We'll use my "tape delay" later
  delay1.delay(0, 250);
  delay2.delay(0, 20);

  //The mixer has four inputs we can change the volume of
  // gain.(channel from 0 to 3, gain from 0.0 to a large number)
  // A gain of 1 means the output is the same as the input.
  // .5 would be half volume and 2 would be double
  // -1.0 would mean the same volume but the signal is upside down, aka 180 degrees out of phase

  //Since we have two oscillators coming in that are already "1" We should take them down by half so we don't clip.
  // but the filter needs some headroom for the resonance so we'll take it down a bit more
  // If you go over "1" The top or bottom of the wave is just slammed against a wall
  mixer1.gain(0, .2);
  mixer1.gain(1, .2);
  mixer1.gain(2, .2);
  mixer1.gain(3, .2);
  //the other channels of the mixer aren't used so don't need to be set

  //the second mixer is for delay feedback
  mixer2.gain(0, .5);//low pass in
  mixer2.gain(1, 0);//bandpass in
  mixer2.gain(2, 0);//highpass in
  mixer2.gain(3, 0);//signal coming back from the delay

  envelope1.attack(10);
  envelope1.decay(10);
  envelope1.sustain(1);
  envelope1.release(250);



  analogReadAveraging(64); //take lots of readings and average them each time we do analogRead. This is one stage of smoothing

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
    buttons[i].interval(10);              // interval in ms
  }

} //setup is over


void loop() {
  current_time = millis();

  //melody_speed = 1000;
  melody_speed = (analogRead(A10) / 1023.0) * 500;
  if (current_time - prev_time[2] > melody_speed) { //step through the sequnce, the meoldy
    prev_time[2] = current_time;

    melody_location++; //melody_location = melody_location + 1
    if (melody_location > 7) {
      melody_location = 0;
    }

    int temp1 = melody1[melody_location];

    if (temp1 > 0) {
      int random_enable = random(100);
      int random_note1 = random(2, 13); //2,3,4....11,12
      if (random_enable > 75 && melody_location > 3) {
        temp1 = temp1 +  random_note1;
      }
      melody_note_number = temp1;
      new_note_flag = 1;
      melody_note_timer = current_time;
    }

  }

  gate_length = 50;
  if (current_time - melody_note_timer > gate_length) { //after 50 millis, turn the env off
    new_note_flag = 0;
  }



  for (int j = 0; j < NUM_BUTTONS; j++)  {
    buttons[j].update();
    if (buttons[j].fell()) {
      new_note_flag = 1;
      new_note_number = j;
    }

    if ( buttons[j].rose() ) {
      new_note_flag = 0;
    }
  }

  int offset1 = (analogRead(A16) / 1023.0) * 100;
  int note_select = (new_note_number * 5) + offset1 ;
  note_select = constrain(note_select, 0, 120);

  float fm1 = (lfo[0] * .0025) + 1.0;

  freq1 = chromatic[melody_note_number];

  spacing1 = (analogRead(A11) / 1023.0) * 3.0;

  waveform1.frequency(freq1);
  waveform2.frequency(freq1 * spacing1);
  waveform3.frequency(freq1 * 2.0);
  waveform4.frequency(freq1 * 0.5);

  if (new_note_flag == 1) {
    envelope1.noteOn();
    new_note_flag = 2;
  }
  if (new_note_flag == 0) {
    envelope1.noteOff();
    new_note_flag = 2;
  }

  float amp1 = 1.0 - ( analogRead(A14) / 1023.0);

  int temp1 = analogRead(A13);

  if (temp1 < 333) {
    mixer2.gain(0, amp1);//low pass in
    mixer2.gain(1, 0);//bandpass in
    mixer2.gain(2, 0);//highpass in
  }
  if (temp1 >= 333 && temp1 < 666) {
    mixer2.gain(0, 0);//low pass in
    mixer2.gain(1, amp1);//bandpass in
    mixer2.gain(2, 0);//highpass in
  }
  if (temp1 >= 666) {
    mixer2.gain(0, 0);//low pass in
    mixer2.gain(1, 0);//bandpass in
    mixer2.gain(2, amp1);//highpass in
  }

  //use three different things to adjust the filter frequency
  float filter_freq_pot = map(analogRead(A12), 0, 1023, 100, 15000); // a pot that has a range of 100 to 15k
  float lfo_mod = .5 + (lfo[0] * .5); //lfo reading which now goes from .5 to 1.0
  float filter_freq = freq1 + (filter_freq_pot * lfo_mod); //and the current note frequency
  filter1.frequency(filter_freq);

  float amp2 = 1.0 - (analogRead(A15) / 1023.0);
  mixer2.gain(3, amp2); //feedback

  waveform5.frequency(analogRead(A17) / 100.0);

  if (peak1.available() == 1) { //to read the current highest level of the waveform attached to peak1 we first have to check if it's ready
    lfo[0] = peak1.read(); //returns a 0-1.0
  }

  if (current_time - prev_time[0] > 20 && 1) {
    prev_time[0] = current_time;

    Serial.println(filter_freq);
  }

  if (current_time - prev_time[0] > 500 && 0) { //change "&& 0" to "&& 1" to print this
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
