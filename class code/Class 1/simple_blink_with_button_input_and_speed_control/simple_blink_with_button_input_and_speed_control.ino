//using a button turn on and off the blinking
// and a potentiometer to control the speed of the blinking
// See "simple blink" in the class 1 folder for more notes

int led_output = 0;
unsigned long current_time;
unsigned long prev_time = 0;
int led_pin = 13; //built in led on the Teensy 4.1
int button_pin = 30; //on the bleep base the left most button is pin 30
int button_state;
int prev_button_state;
int blinky_state;
int pot_pin=A10; //the top left pot is A10 on the bleep base
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


  //the button might "bounce" so sometimes when you press it it might go back and forth very quickly
  // we'll fix this later
  if (prev_button_state == 1 && button_state == 0) {
    if (blinky_state == 1) {
      blinky_state = 0;
    }
    else {
      blinky_state = 1;
    }
  }

  if (blinky_state == 0) {
    if (current_time - prev_time > pot_reading) { //no instead of a "hard coded" value for the rate we use a variable we can change with the top
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
