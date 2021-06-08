#define LED_pin 13

void setup() {
  // put your setup code here, to run once:
  analogWriteResolution(12);
  analogWriteFrequency(LED_pin, 36621.09); //12 bit max feq for teensy4.1
  randomSeed(939);
}

void loop() {
  int pot1 = analogRead(A10)*4; //10 bit to 12 bit
  analogWrite(LED_pin,pot1);
 
}
