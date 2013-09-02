
#define SND_SENSOR_PIN 2
#define LED 13

boolean flag = false;

void setup() {
  pinMode(SND_SENSOR_PIN, INPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  
  if( digitalRead(SND_SENSOR_PIN) == LOW )
  {
    Serial.println("Sound detected!");
    flag = !flag;
    digitalWrite(LED, flag ? HIGH : LOW);
    delay(200);
  }
  
}
