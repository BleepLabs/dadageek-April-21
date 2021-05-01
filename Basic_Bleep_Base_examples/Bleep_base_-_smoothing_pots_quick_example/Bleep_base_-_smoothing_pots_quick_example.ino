//a quick example on how to smooth pot readings 


int smoothed; int raw;
unsigned long current_time;
unsigned long prev_time[8]; //an array of 8 variables, all named prev_time


void setup() {
  analogReadResolution(10); //teensy 4.x is 10 bit, 0-1023
  analogReadAveraging(64); //take lots of readings and average them each time we do analogRead. This is one stage of smoothing
} //setup is over

void loop() {
  current_time = millis();

  //one way to smooth readigns is to slow down how often you take them a tiny bit

  if (current_time - prev_time[1] > 2) {
    prev_time[1] = current_time;
    raw = analogRead(A10);
    //smoothAnalogRead(array select, input)
    // array select is an a seperate value of reach pot. On the top left use 0 and then 1 for the next one and so on
    // input is the number we want to smooth, the raw analogread
    smoothed = smoothAnalogRead(0, raw);
  }

  if (current_time - prev_time[0] > 20) { //2 millis is too fast to print so we slow the printing down more
    prev_time[0] = current_time;
    Serial.print(raw); //"print" has no return, or new line, after it
    Serial.print(" "); //print a space
    Serial.println(smoothed); //"println" for an return
  }
}// loop is over

//copy paste all of this for smoother analogreads at the expense of a tiny bit of lag
//based on https://playground.arduino.cc/Main/DigitalSmooth/

#define filterSamples   13   // filterSamples should  be an odd number, no smaller than 3. Incerease for more smoooothness
#define array_num 8 //numer of differnt smooths we can take, one for each pot
int sensSmoothArray[array_num] [filterSamples];   // array for holding raw sensor values for sensor1

int smoothAnalogRead(int array_sel, int input) {    // "int *sensSmoothArray" passes an array to the function - the asterisk indicates the array name is a pointer
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
