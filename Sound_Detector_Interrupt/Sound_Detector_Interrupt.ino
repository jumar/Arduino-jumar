//SND is connected on pin 2 (external interrupt 0)
//and the LED will be flipped when the interrupt is raised
//SND will go LOW when sound is detected
#define SND_PIN  2
#define LED 13

volatile int state = LOW;

void setup()
{
  pinMode(LED, OUTPUT);
  attachInterrupt(0, blink, FALLING);
}

void loop()
{
  digitalWrite(LED, state);
}

void blink()
{
  state =! state;
}

