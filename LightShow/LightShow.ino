// When motion is detected the strand is turned ON
// in low/slow mode
// when sound is detected it goes wild
// when no motion is detected during 15s it goes low/slow mode
// when no motion is detected during 30s it turns off

#include "TimerOne.h"
#include <Adafruit_NeoPixel.h>

#define SERIAL_TRACE

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
#define NO_MVT_SLEEP_DELAY 60000
//timeout in ms before shuting down
#define NO_MVT_SHUTDOWN_DELAY 120000
//time in ms between two snd events
#define SND_DELAY 500 

//brightness of strand during light show
#define HI_BRIGHTNESS 255
//brightness of strand during standby mode
#define LO_BRIGHTNESS 16

//number of lighting schemes
#define NB_SHOW 6

volatile int sndState = LOW; 
volatile int PIRState = LOW;
volatile int PIRVal = 0;
volatile unsigned long lastPIRTime = 0;
volatile unsigned long lastSNDTime = 0;
volatile int showType = 0;
volatile boolean _break = false;

#define SHUTDOWN 0
#define SLEEP 1
#define LIGHTSHOW 2
#define WHITE_LIGHT 3
#define IDLE_ 4

int state = LIGHTSHOW;
int lastState = state;
int ledState = LOW;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(149, STRAND_PIN, NEO_GRB + NEO_KHZ800);

// defines for serial commands
#define NO_ACTION 245
#define SND_OFF 'a'
#define SND_ON 'b'
#define MVT_OFF 'c'
#define MVT_ON 'd'
#define NO_SLEEP 'e'
#define ALLOW_SLEEP 'f'
#define WHITE_LIGHT_ON 'g'
#define LIGHTSHOW_CMD 'h'
#define SHUTDOWN_CMD 'i'
byte fromSerial = NO_ACTION;

void setup()
{
  pinMode(LED, OUTPUT);
  lastPIRTime = lastSNDTime = millis();
  enable_SND(true);
  enable_MVT(true);
  Timer1.initialize(1000000); // Set period to 1s
  Timer1.attachInterrupt(timerCallback);  // attaches timerCallback() as a timer overflow interrupt
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);
  #ifdef SERIAL_TRACE
    Serial.println("Go");
  #endif
}

void enable_SND(boolean activate)
{
  if(activate)
    attachInterrupt(0, sound, FALLING);
  else
    detachInterrupt(0);
}

void enable_MVT(boolean activate)
{
  if(activate)
    attachInterrupt(1, movement, CHANGE);
  else
    detachInterrupt(1);
}

void enableSleepTimer(boolean enable)
{
  if(enable)
    Timer1.restart();
  else
    Timer1.stop();
}

void serialEvent()
{
  fromSerial = Serial.read();
  #ifdef SERIAL_TRACE
    Serial.print("Rcv:");
    Serial.println(fromSerial);
  #endif
  switch(fromSerial)
  {
    case SND_OFF:
      enable_SND(false);
    break;
    case SND_ON:
      enable_SND(true);
    break;
    case MVT_OFF:
      enable_MVT(false);
    break;
    case MVT_ON:
      enable_MVT(true);
    break;
    case NO_SLEEP:
      enable_Sleep(false);
    break;
    case ALLOW_SLEEP:
      enable_Sleep(true);
    break;
    case WHITE_LIGHT_ON:
      setState(WHITE_LIGHT);
    break;
    case SHUTDOWN_CMD:
      setState(SHUTDOWN);
    break;
    case LIGHTSHOW_CMD:
      setState(LIGHTSHOW);
    break;
  }
}

void enable_Sleep(boolean enable)
{
  if(enable)
  {
   lastPIRTime = millis();
  }
  if(!enable && (state == SLEEP || state == SHUTDOWN))
  {
    setState(lastState);
  }
  enableSleepTimer(enable);
}

void setState(int newState)
{
  if(newState != state)
  {
    if(state = SLEEP)
      digitalWrite(LED, LOW); // turn off blinking LED
    if(state == LIGHTSHOW || state == WHITE_LIGHT)
      lastState = state;
    state = newState;
    _break = true;
    #ifdef SERIAL_TRACE
      Serial.print("new State: ");
      Serial.println(newState);
    #endif
  }
}

void loop()
{
  switch(state)
  {
    default:
    case SHUTDOWN:
      setColor(strip.Color(0, 0, 0));
       //turn the LED off while sleeping
      ledState = !ledState;
      digitalWrite(LED, LOW);
    break;
    case SLEEP:
      strip.setBrightness(LO_BRIGHTNESS);
      strip.show();
      //blink the LED while sleeping
      ledState = !ledState;
      digitalWrite(LED, ledState);
      delay(500);
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
    case WHITE_LIGHT:
      strip.setBrightness(HI_BRIGHTNESS);
      colorFull(strip.Color(254, 254, 254), 0);
      strip.show();
      setState(IDLE_);
    break;
    case IDLE_:
    // do nothing!
    break;
  }
  //STATE MACHINE
}

void movement()
{
  PIRVal = digitalRead(PIR_PIN); 
  if (PIRVal == HIGH) 
  { 
    if (PIRState == LOW) // we have just turned on
    { 
      #ifdef SERIAL_TRACE
        Serial.println("Mvt!");
      #endif
      PIRState = HIGH;
      // get out of sleep/shutdown mode
      setState(lastState);
    }  
  } 
  else 
  {    
    if (PIRState == HIGH) // we have just turned of
    {
      //Serial.println("Motion ended!");      // We only want to print on the output change, not state
      PIRState = LOW;
      lastPIRTime = millis(); // we memorize the time of the end of the last movement
    }
  }
  digitalWrite(LED, PIRState);
}

void sound()
{
  // SND sensor can generate multiple successive interrupts, we allow only one each 500ms
  unsigned long elapsedSinceLastSnd = millis() - lastSNDTime;
  if(elapsedSinceLastSnd > SND_DELAY) 
  {
    //We had sound -> change show type
    showType++;
    //break out of loops
    _break = true;
    lastSNDTime = millis();
    #ifdef SERIAL_TRACE
        Serial.println("Snd!");
    #endif
  }
  // a sound is treated as a movement to reset shutdown countdown
  lastPIRTime = millis();
}


void timerCallback()
{
  unsigned long elapsedSinceLastMvt = millis() - lastPIRTime;
  if(elapsedSinceLastMvt > NO_MVT_SHUTDOWN_DELAY)
  {
    if(state != SHUTDOWN)
    {
      #ifdef SERIAL_TRACE
        Serial.println("Shuting Down ...");
      #endif
      setState(SHUTDOWN);
    }
  }
  else if(elapsedSinceLastMvt > NO_MVT_SLEEP_DELAY)
  {
    if(state != SLEEP)
    {
      #ifdef SERIAL_TRACE
        Serial.println("Sleep Mode ZZzzZZ");
      #endif
      setState(SLEEP);
    }
  }
  else
  {
    if(state == SLEEP || state == SHUTDOWN)
    {
      setState(lastState);
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
  if(wait>0)
  {
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
  }
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
