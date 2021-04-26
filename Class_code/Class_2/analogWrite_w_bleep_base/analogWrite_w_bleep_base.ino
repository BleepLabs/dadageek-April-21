//make light blink at variable rate controled by pot
//light should increase and decrease at a changeabe rate

unsigned long prev_time;
unsigned long current_time;
int led_pin = 13;
int rate1 = 500;
int led_state;
int lfo;
int lfo_latch;

void setup() {
  pinMode(led_pin, OUTPUT);
}

void loop() {
  current_time = millis();

  rate1 = (analogRead(A10) / 2); //0-1023

  if (current_time - prev_time > rate1) {
    prev_time = current_time;

    if (lfo_latch == 1) {
      lfo = lfo + 1;
    }
    if (lfo_latch == 0) {
      lfo = lfo - 1;
    }

    if (lfo > 255) {
      lfo = 255;
      lfo_latch = 0;
    }
    if (lfo < 0) {
      lfo = 0;
      lfo_latch = 1;
    }
    analogWrite(led_pin, lfo); //0-255
  }



}
