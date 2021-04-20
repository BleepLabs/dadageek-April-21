//using a button tun on and off the blinking
// See "simple blink" in the class 1 folder for more notes


int led_output = 0;
unsigned long current_time;
unsigned long prev_time = 0;
int led_pin = 13;
int button_pin = 30; //on the bleep base the left most button is pin 30
int button_state;
int prev_button_state;
int blinky_state;

void setup() {
  pinMode(led_pin, OUTPUT);

  //to use a button we need to set the pin to INPUT_PULLUP
  // since the button is just connected to ground, we need to have the Teensy attach a resistor between the pin and 3.3V so there is no ambiguity
  // otherwise when the button is not being pressed it's just "floating". Digital only can understand on or off.
  pinMode(button_pin, INPUT_PULLUP); 
}

void loop() {
  current_time = millis();

  prev_button_state = button_state; //remember what button_state was last loop
  button_state = digitalRead(button_pin); //update button_state

  //no matter how fast we're reading the button, there will always be a time when the button
  // was not being pressed one loop and then the next loop it is pressed.
  // && means also (a singe & is a different thing). THis means both these statements must be true to execute the code inside the {}
  
  //the button might "bounce" so sometimes when you press it it might go back and forth very quickly and not have the desired outcome
  // we'll fix this later
  if (prev_button_state == 1 && button_state == 0) {
    if (blinky_state == 1) { //same method as was used to toggle the LED 
      blinky_state = 0;
    }
    else {
      blinky_state = 1;
    }
  }

  if (blinky_state == 0) { //only do what's inside if the state is 0
    if (current_time - prev_time > 100) { //then it's the same timing "if"
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
