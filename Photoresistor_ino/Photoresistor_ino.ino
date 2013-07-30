/***********************************
            PhotoR     10K
 +5    o---/\/\/--.--/\/\/---o GND
                  |
 Pin 0 o-----------
************************************/

int lPhotoPin = 0;
int lPhotoValue = 0;

int lLEDPin = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin( 9600 );
  pinMode( lLEDPin, OUTPUT );
}

void loop() {
  // put your main code here, to run repeatedly: 
  lPhotoValue = analogRead( lPhotoPin );
  
  //Serial.print( "Analog reading = " );
  //Serial.println( lPhotoValue );     // the raw analog reading
  digitalWrite( lLEDPin, HIGH );
  delay( lPhotoValue );
  digitalWrite( lLEDPin, LOW );
  delay( lPhotoValue );
}
