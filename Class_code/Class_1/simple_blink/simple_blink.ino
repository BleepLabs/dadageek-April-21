//Blink the built in light at a set rate

//This top part of the the "sketch" is the initialization section
// It's where we can make new variables by defining their type and naming them

//"int" is an integer that can hold numbers from -32768 to 32767 which is 16 bits
// "unsigned long" is an integer that can hold 0 to 4,294,967,295 which is 32 bits
int led_output = 0; //makes as a new integer called led_output and sets it equal to 0
unsigned long current_time; //if we don't tell it what to be it's also set to 0
unsigned long prev_time = 0;
int led_pin = 13; //led pin on pretty much every Arduino device

//pretty much every line of code ends in a ";"


//you can initialize variables anywhere but we'll keep it simple for not and just do them above the setup
// setup is code that is run just once when the device turns on or reboots
void setup() { //it begins with this curly bracket...
  
  //set the led_pin as an output so we can use it to blink an LED
  // https://www.arduino.cc/reference/en/language/functions/digital-io/pinmode/
  pinMode(led_pin, OUTPUT); 

}// ...and ends with this one. Curly brackets contain and separate different sections of code

void loop() {//loop begins. This section runs as fast as it can, repeating everything in it in order
  //"millis" returns the amount of milliseconds that have elapsed since the device was turned on or reset
  // we use current_time to hold this value
  current_time = millis();

  //this section of code will be run ever 250 milliseconds
  // prev_time is used to remember the last time we ran the code. 
  // It starts out as 0, then the first time the code between the {} is run it copies whatever value is in current_time which would be 251. 
    
  if (current_time - prev_time > 250) {
    prev_time = current_time;

//and "if" followed by an "else" allows us to have a logical statement with just two outcomes
// if led_output and 1 are the same value then the section in the first {} is run
// if they are not the same then the second part after the "else" is run
    if (led_output == 1) { 
      led_output = 0;
    }
    else {
      led_output = 1;
    }
    //output either a high or low, 0 or 1, 0V or 3.3V out of the pin indicated by led_pin
    // Our Teensy has enough power to light up a tiny LED but it couldn't turn on something like an LED bulb
    // it puts out a max of 10 milliamps per pin which means 3.3V * 10mA = 0.033 Watts
    digitalWrite(led_pin, led_output);

  } // the timing "if" is over

} // the loop is over so the code at the top of the loop starts executing again
