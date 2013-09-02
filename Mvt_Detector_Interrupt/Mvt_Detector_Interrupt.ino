//PIR is connected on pin 3 (external interrupt 1)
//and the LED will be flipped when the interrupt is raised
//PIR will go LOW when motion is detected
#define PIR_PIN  3
#define LED 13

volatile int state = LOW;

void setup()
{
  pinMode(LED, OUTPUT);
  attachInterrupt(1, blink, FALLING);
}

void loop()
{
  digitalWrite(LED, state);
}

void blink()
{
  state =! state;
}

