// When motion is detected the strand is turned ON
// in low/slow mode
// when sound is detected it goes wild
// when no motion is detected during 15s it goes low/slow mode
// when no motion is detected during 30s it turns off

#include "TimerOne.h"

//SND is connected on pin 2 (external interrupt 0)
//and the LED will be flipped when the interrupt is raised
//SND will go LOW when sound is detected
#define SND_PIN  2
//PIR is connected on pin 3 (external interrupt 1)
//and the LED will be flipped when the interrupt is raised
//PIR will go LOW when motion is detected
#define PIR_PIN  3

// LED will be used as movement status indicator: ON meaning movement
#define LED 13

//timeout in ms before going in sleep mode
#define NO_MVT_SLEEP_DELAY 15000
//timeout in ms before shuting down
#define NO_MVT_SHUTDOWN_DELAY 30000

volatile int sndState = LOW; 
volatile int PIRState = LOW;
volatile int PIRVal = 0;
volatile unsigned long lastPIRTime = 0;
volatile int showType = 0;

#define SHUTDOWN 0
#define SLEEP 1
#define LIGHTSHOW 2

int state = SHUTDOWN;
int ledState = LOW;

void setup()
{
  pinMode(LED, OUTPUT);
  attachInterrupt(0, sound, FALLING);
  attachInterrupt(1, movement, CHANGE);
  Timer1.initialize(1000000); // Set period to 1s
  Timer1.attachInterrupt(timerCallback);  // attaches timerCallback() as a timer overflow interrupt
  //lastPIRTime = millis();
  Serial.begin(9600);
  Serial.println("Go");
}

void loop()
{
  switch(state)
  {
    default:
    case SHUTDOWN:
      digitalWrite(LED, LOW);
    break;
    case SLEEP:
      digitalWrite(LED, HIGH);
    break;
    case LIGHTSHOW:
      ledState = !ledState;
      digitalWrite(LED,ledState);
      for(int i = 0; i< showType%5 + 1; i++)
        delay(200);
    break;
  }
  
  //STATE MACHINE
}

void movement()
{
  PIRVal = digitalRead(PIR_PIN); 
  if (PIRVal == HIGH) 
  { 
    if (PIRState == LOW) 
    {      // we have just turned on
      //Serial.println("Motion detected!");      // We only want to print on the output change, not state      
      PIRState = HIGH;
      //runShow();
      state = LIGHTSHOW;
    }  
  } 
  else 
  {    
    if (PIRState == HIGH)
    {      // we have just turned of
      //Serial.println("Motion ended!");      // We only want to print on the output change, not state
      PIRState = LOW;
      lastPIRTime = millis(); // we memorize the time of the end of the last movement
    }
  }
  //digitalWrite(LED, PIRState);
}

void sound()
{
  //Serial.println("Sound !");
  //change light-show type
  showType++;
}


void timerCallback()
{
  unsigned long elapsedSinceLastMvt = millis() - lastPIRTime;
  //Serial.print("elapsed time: ");
  //Serial.print(elapsedSinceLastMvt);
  //Serial.println("ms");
  if(elapsedSinceLastMvt > NO_MVT_SHUTDOWN_DELAY)
  {
    if(state != SHUTDOWN)
    {
      Serial.println("Shuting Down"); 
      state = SHUTDOWN;
    }
  }
  else if(elapsedSinceLastMvt > NO_MVT_SLEEP_DELAY)
  {
    if(state != SLEEP)
    {
      Serial.println("Sleep Mode ZZzzZZ"); 
      state = SLEEP;
    }
  }
  else
  {
    if(state != LIGHTSHOW)
    {
      state = LIGHTSHOW;
    }
  }
}

