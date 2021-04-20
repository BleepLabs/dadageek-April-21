int led_output = 0;
unsigned long current_time;
unsigned long prev_time = 0;
int led_pin = 0;
int button_pin = 12;
int button_state;
int prev_button_state;
int blinky_state;
int pot_pin=A0;
int pot_reading; 

void setup() {
  pinMode(led_pin, OUTPUT);
  pinMode(button_pin, INPUT_PULLUP);
}

void loop() {
  current_time = millis();

  prev_button_state = button_state;
  button_state = digitalRead(button_pin);

  pot_reading = analogRead(pot_pin); //returns a value between 0 - 1023 aka 10 bits

  if (prev_button_state == 1 && button_state == 0) {
    if (blinky_state == 1) {
      blinky_state = 0;
    }
    else {
      blinky_state = 1;
    }
  }

  if (blinky_state == 0) {
    if (current_time - prev_time > pot_reading) {
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
}
