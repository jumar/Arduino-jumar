#include <WebSocketClient.h>

#include <SPI.h>
#include <Ethernet.h>
#include "TimerOne.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0x01 };
byte ip[]  = { 192, 168, 1, 101 };

char server[] = "192.168.1.3";
char echoPath[] = "/echo";
char playPath[] = "/mediaplayer/play";
char stopPath[] = "/mediaplayer/stop";
int port = 8080;

char track0[] = "0";
char track1[] = "1";

WebSocketClient echoSocket;
WebSocketClient playSocket;
WebSocketClient stopSocket;

volatile boolean pongRecieved = true;
//timeout in ms before considering that the server is down
#define TIMER_PERIOD 4000000
int ledRed = 13;
int ledGreen = 11;
int ledBlue = 12; 

void setup() {
  // initialize pins for LEDS.
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledBlue, HIGH);
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.println("Initialising ethernet client");
  // start the Ethernet connection:
  Ethernet.begin(mac, ip);
 
  // print your local IP address:
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++)
  {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print("."); 
  }
  Serial.println();
  Serial.println("Connecting to test server");
  // connect to server
  echoSocket.connect(server, echoPath, port);
  echoSocket.setDataArrivedDelegate(dataArrived);
  playSocket.connect(server, playPath, port);
  stopSocket.connect(server, stopPath, port);
  
  // setup timer for server montoring
  Timer1.initialize(TIMER_PERIOD); // Set period to 1s
  Timer1.attachInterrupt(timerCallback);  // attaches timerCallback() as a timer overflow interrupt
}

void loop() 
{
  echoSocket.monitor();
}

void mPlay(char * pTrackNb)
{
  playSocket.send(pTrackNb);
}

void mStop()
{
  stopSocket.send("");
}

void mPing()
{
  Serial.println("Ping");
  pongRecieved = false;
  echoSocket.send("ping");
}

void dataArrived(WebSocketClient client, String data) 
{
  pongRecieved = true;
  Serial.println("Pong: " + data);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(ledBlue, LOW);
}

void timerCallback() // called every 1s
{
  
  // TODO refactor interrupt code to be as fast as possible
  // we should only set flags here
  if(!pongRecieved)
  {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledBlue, LOW);
    //mStop();
    if(!echoSocket.connected())
    {
      Serial.println("Connection lost, re-connecting");
      echoSocket.disconnect();
      echoSocket.connect(server, echoPath, port);
      echoSocket.setDataArrivedDelegate(dataArrived);
    }
    else
    {
       Serial.println("Timeout but Connection Not lost...");
    }
  }
  else
  {
    mPlay(track1);
  }
  mPing();
}
