//make light blink at variable rate controled by pot

unsigned long prev_time;
unsigned long current_time;
int led_pin = 13;
int rate1 = 500;
int led_state;

void setup() {
  pinMode(led_pin, OUTPUT);
}

void loop() {
  current_time = millis();

  rate1 = (analogRead(A10) / 2) + 100; //0-1023

  if (current_time - prev_time > rate1) {
    prev_time = current_time;

    if (led_state == 1) {
      led_state = 0;
    }
    else {
      led_state = 1;
    }
    digitalWrite(led_pin, led_state);
  }

}
