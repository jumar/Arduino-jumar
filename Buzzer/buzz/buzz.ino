int buzzerPin = 9;   
int i = 0;
void setup()  { 
  // nothing happens in setup 
} 

void loop()  { 
   
    // 127 means 50% duty cycle
    analogWrite(buzzerPin, 127);         
    delay(i++%100);                            
 
}
