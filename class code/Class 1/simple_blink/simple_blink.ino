int led_output = 0;
unsigned long current_time;
unsigned long prev_time = 0;
int led_pin = 0;

void setup() {
  pinMode(led_pin, OUTPUT);
}

void loop() {
  current_time = millis();

  if (current_time - prev_time > 250) {
    prev_time = current_time;

    if (led_output == 1) {
      led_output = 0;
    }
    else {
      led_output = 1;
    }
    digitalWrite(led_pin, led_output);

  }

}
