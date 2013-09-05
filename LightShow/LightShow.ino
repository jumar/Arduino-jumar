// When motion is detected the strand is turned ON
// in low/slow mode
// when sound is detected it goes wild
// when no motion is detected during 15s it goes low/slow mode
// when no motion is detected during 30s it turns off

#include "TimerOne.h"
#include <Adafruit_NeoPixel.h>

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

//Light strip on pin 10
#define STRAND_PIN 10

//timeout in ms before going in sleep mode
#define NO_MVT_SLEEP_DELAY 15000
//timeout in ms before shuting down
#define NO_MVT_SHUTDOWN_DELAY 30000

//brightness of strand during light show
#define HI_BRIGHTNESS 255
//brightness of strand during standby mode
#define LO_BRIGHTNESS 16

#define NB_SHOW 6

volatile int sndState = LOW; 
volatile int PIRState = LOW;
volatile int PIRVal = 0;
volatile unsigned long lastPIRTime = 0;
volatile int sndCounter = 0;
volatile int showType = 0;
volatile boolean _break = false;

#define SHUTDOWN 0
#define SLEEP 1
#define LIGHTSHOW 2

int state = SHUTDOWN;
int ledState = LOW;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(149, STRAND_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  pinMode(LED, OUTPUT);
  attachInterrupt(0, sound, FALLING);
  attachInterrupt(1, movement, CHANGE);
  Timer1.initialize(1000000); // Set period to 1s
  Timer1.attachInterrupt(timerCallback);  // attaches timerCallback() as a timer overflow interrupt
  //lastPIRTime = millis();
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
  Serial.println("Go");
}

void loop()
{
  // check for sound
  if(sndCounter != 0)
  {
    //We had sound -> change show type
    showType++;
    sndCounter = 0;
  }
  switch(state)
  {
    default:
    case SHUTDOWN:
      setColor(strip.Color(0, 0, 0));
      digitalWrite(LED, LOW);
    break;
    case SLEEP:
      strip.setBrightness(LO_BRIGHTNESS);
      strip.show();
      //blink the LED
      ledState = !ledState;
      digitalWrite(LED, ledState);
      delay(800);
    break;
    case LIGHTSHOW:
      strip.setBrightness(HI_BRIGHTNESS);
      strip.show();
      //showType = 3;
      switch(showType%(NB_SHOW+1))
      {
        default:
        case 0:
          if(colorWipe(strip.Color(0, 0, 255), 25, false)) // Blue
            colorWipe(strip.Color(0, 0, 0), 25, true);
          break;
        case 1:
          rainbow(40);
          break;
        case 2:
          if(colorWipe(strip.Color(255, 0, 0), 25, false)) // Red
            colorWipe(strip.Color(0, 0, 0), 25, true);
          break;
        case 3:
          if(colorFull(Wheel(50), 1000))
            if(colorFull(Wheel(150), 1000))
              if(colorFull(Wheel(200), 1000))
                colorFull(Wheel(250), 1000);
          break;
        case 4:
          if(colorWipe(strip.Color(0, 255, 0), 25, false)) // Green
            colorWipe(strip.Color(0, 0, 0), 25, true);
          break;
        case 5:
          rainbowCycle(40);
          break;
        case 6:
          if(colorFull(Wheel(10), 200))
            if(colorFull(Wheel(57), 200))
              if(colorFull(Wheel(123), 200))
                colorFull(Wheel(213), 200);
          break;
      }
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
  digitalWrite(LED, PIRState);
}

void sound()
{
  //Serial.println("Sound !");
  //change light-show type
  sndCounter++;
  _break = true;
  timerCallback();
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
      //Serial.println("Shuting Down"); 
      state = SHUTDOWN;
    }
  }
  else if(elapsedSinceLastMvt > NO_MVT_SLEEP_DELAY)
  {
    if(state != SLEEP)
    {
      //Serial.println("Sleep Mode ZZzzZZ"); 
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

//Set givn color to all leds
void setColor(uint32_t c)
{
   for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}

// Fill the dots one after the other with a color
boolean colorWipe(uint32_t c, uint8_t wait, boolean reverse) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(reverse?strip.numPixels()-i:i, c);
      strip.show();
      delay(wait);
      if(_break)
      {
        _break = false;
        return false;
      }
  }
  return true;
}

boolean colorFull(uint32_t c, unsigned long wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
  unsigned long start = millis();
  do
  {
    delay(1);
    if(_break)
    {
      _break = false;
      return false;
    }
  }
  while(millis()-start<wait);
  return true;
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
    if(_break)
    {
      _break = false;
      break;
    }
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
    if(_break)
    {
      _break = false;
      break;
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
