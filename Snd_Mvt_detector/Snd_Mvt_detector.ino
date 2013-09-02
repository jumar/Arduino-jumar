
#define SND_SENSOR_PIN 2
#define PIR_PIN  6
#define LED 13

int val = 0;  // variable to store the value coming from the sensor
boolean flag = true;
int pirState = LOW; 

void setup() {
  pinMode(SND_SENSOR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  
  if( digitalRead(SND_SENSOR_PIN) == LOW )
  {
    Serial.println("Sound detected!");
    flag = !flag;
    //digitalWrite(LED, flag ? HIGH : LOW);
    //delay(200);
  }
  val = digitalRead(PIR_PIN);  // read input value  
  if (val == HIGH) 
  {            // check if the input is HIGH    
    if (pirState == LOW) 
    {      // we have just turned on
      digitalWrite(LED, HIGH);
      Serial.println("Motion detected!");      // We only want to print on the output change, not state      
      pirState = HIGH;    
    }  
  } 
  else 
  {    
    if (pirState == HIGH)
    {      // we have just turned of
      digitalWrite(LED, HIGH);
      Serial.println("Motion ended!");      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
  
}
