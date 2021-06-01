
unsigned int SD2RAM(char* fn, uint16_t* dest_array) {
#define printer 1
  byte sample_num;
  uint32_t filelength;
  int32_t dest_array_size = sizeof(dest_array);
  uint32_t dest_loc;
  int data_start;
  int real_len;

  File root = SD.open("/");

  File filetocopy =  SD.open(fn);
  if (! filetocopy) {
    Serial.println(fn);
    Serial.println(" is NOT on the SD card");
  }

  else {
    if (printer) {
      Serial.println(fn);
      Serial.println(" is on the SD card");
    }
    const char *filename = filetocopy.name();

    const byte wav_header_size = 200;
    const int buffsize = 1024;

    byte b1[buffsize];
    byte b2[buffsize * 2];
    if (filetocopy.available()) {
      filetocopy.seek(0);
      filetocopy.read(b1, wav_header_size);
    }
    int e1 = 0;
    for (int j = 0; j < wav_header_size; j++) {
      /*
        Serial.print(b1[j], HEX);
        Serial.print(" ");
        e1++;
        if (e1 > 23) {
          e1 = 0;
          Serial.println();
        }
      */
      if (b1[j] == 0x64 && b1[j + 1] == 0x61 && b1[j + 2] == 0x74 && b1[j + 3] == 0x61) {
        data_start = j + 8;
        real_len = (b1[j + 7] << 24) | (b1[j + 6] << 16) | (b1[j + 5] << 8) | b1[j + 4];
        if (printer) {
          Serial.print("data_start ");
          Serial.println(data_start);
          Serial.print("real_len ");
          Serial.println(real_len);
        }
      }
    }
    byte bits_per_sample = b1[34];
    unsigned long filelength = filetocopy.size();
    byte num_channels = b1[22];
    if (printer) {
      Serial.print("num_channels ");
      Serial.println(num_channels);

      Serial.print("bits per sample ");
      Serial.println(bits_per_sample);

      Serial.print("file length ");
      Serial.println(filelength);
    }
    if (num_channels == 1) {
      for (uint32_t i = data_start; i < filelength; i += 512) {
        if (!filetocopy.available()) {
          // Serial.print("na ");
        }
        filetocopy.seek(i);
        filetocopy.read(b1, 512);
        for (int j = 0; j < 512; j += 2) {
          dest_array[dest_loc] = ((b1[j + 1] << 8) | b1[j]);
          dest_loc++;
        }
      }
    }
    else {
      Serial.print("only mono, 16b samples are currently working");
    }

    filetocopy.close();
  }
  root.close();
  return dest_loc;
  delay(10);

}



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
  //keep subtracting or adding 1 until it's in the range of 0-1.0
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

#define filterSamples   17   // filterSamples should  be an odd number, no smaller than 3. Increase for more smoooothness
#define array_num 8 //number of different smooths we can take, one for each pot
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
