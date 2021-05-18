/*
Parallel delays with an envelope 
One delay uses a filter in the feedback path
*/

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioInputI2S            i2s2;           //xy=91,325.7500057220459
AudioEffectEnvelope      envelope1;      //xy=266.25000381469727,360.0000057220459
AudioMixer4              input_mixer1;         //xy=453.75000762939453,418.25000762939453
AudioEffectDelay         delay1;         //xy=453.50000762939453,529.0000076293945
AudioEffectDelay         delay2;         //xy=490.00000762939453,217.5
AudioMixer4              input_mixer2;         //xy=495,77.5
AudioFilterStateVariable filter1;        //xy=643.7500114440918,193.75000381469727
AudioMixer4              output_mixer;         //xy=832.0000095367432,342.50000381469727
AudioOutputI2S           i2s1;           //xy=1056.2500114440918,232.25000381469727
AudioConnection          patchCord1(i2s2, 0, envelope1, 0);
AudioConnection          patchCord2(envelope1, 0, input_mixer1, 0);
AudioConnection          patchCord3(envelope1, 0, input_mixer2, 0);
AudioConnection          patchCord4(envelope1, 0, output_mixer, 1);
AudioConnection          patchCord5(input_mixer1, delay1);
AudioConnection          patchCord6(input_mixer1, 0, output_mixer, 0);
AudioConnection          patchCord7(delay1, 0, input_mixer1, 1);
AudioConnection          patchCord8(delay2, 0, filter1, 0);
AudioConnection          patchCord9(input_mixer2, delay2);
AudioConnection          patchCord10(input_mixer2, 0, output_mixer, 2);
AudioConnection          patchCord11(filter1, 1, input_mixer2, 1);
AudioConnection          patchCord12(output_mixer, 0, i2s1, 0);
AudioConnection          patchCord13(output_mixer, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=268.50000762939453,70.00000190734863
// GUItool: end automatically generated code



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



void setup() {

  analogReadAveraging(64); //take lots of readings and average them each time we do analogRead. This is one stage of smoothing

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  ); //setup the bounce instance for the current button
    buttons[i].interval(10);  // interval in milliseconds. How long after the first cahnge will ignore noise
  }


  AudioMemory(300); //remember to add enough memory blocks! if it's acting weired, see how many its using




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

  input_mixer1.gain(0, .5); //input
  input_mixer1.gain(1, .75); //delay1 feedback

  input_mixer2.gain(0, .5); //input
  input_mixer2.gain(1, .75); //delay1 feedback

  delay2.delay(0, 50);
  filter1.resonance(3.0);


  envelope1.attack(10);
  envelope1.decay(0);
  envelope1.sustain(1.0);
  envelope1.release(10);

  output_mixer.gain(0, .3);
  output_mixer.gain(1, .3);
  output_mixer.gain(2, .3);

  delay1.delay(0, 500);

} //setup is over

void loop() {
  current_time = millis();

  float ff1 = (analogRead(A10) / 1023.0) * 15000.0;
  filter1.frequency(ff1);


  for (int j = 0; j < NUM_BUTTONS; j++)  {
    buttons[j].update();
  }

  if ( buttons[0].fell() ) {
    envelope1.noteOn();

  }

  if ( buttons[0].rose() ) {
    envelope1.noteOff();

  }


  if (current_time - prev_time[0] > 500 && 1) { //cahnge to && 0 to not do this code
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
