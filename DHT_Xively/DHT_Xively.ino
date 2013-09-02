#include <dht.h>
#include <SPI.h>
#include <Ethernet.h>
#include <HttpClient.h>
#include <Xively.h>

dht DHT;

#define POLL_DELAY_MIN 1 
#define DHT22_PIN 5

// MAC address for your Ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Your Xively key to let you upload data
char xivelyKey[] = "tv8AhUiivdNdr6rV61oNO0hxbMfSF20cdQIv3akzXxajOzHZ";

// Define the strings for our datastream IDs
char tempSensorId[] = "Temperature";
char humiditySensorId[] = "Humidity";
XivelyDatastream datastreams[] = {
  XivelyDatastream(tempSensorId, strlen(tempSensorId), DATASTREAM_FLOAT),
  XivelyDatastream(humiditySensorId, strlen(humiditySensorId), DATASTREAM_FLOAT),
};

// Finally, wrap the datastreams into a feed
XivelyFeed feed(879829620, datastreams, 2 /* number of datastreams */);

EthernetClient client;
XivelyClient xivelyclient(client);
void setup()
{
  Serial.begin(115200);
  
  Serial.println("Starting temp and humidity datastream upload to Xively...");
  Serial.print("DHT LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();

  while (Ethernet.begin(mac) != 1)
  {
    Serial.println("Error getting IP address via DHCP, trying again...");
    delay(15000);
  }
  Serial.print("Local IP address: ");
  Serial.println(Ethernet.localIP());
  Serial.println();
}

void loop()
{
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
  // READ DATA
  Serial.print("DHT22, \t");
  int chk = DHT.read22(DHT22_PIN);
  switch (chk)
  {
    case DHTLIB_OK:  
		Serial.print("OK,\t"); 
		break;
    case DHTLIB_ERROR_CHECKSUM: 
		Serial.print("Checksum error,\t"); 
		break;
    case DHTLIB_ERROR_TIMEOUT: 
		Serial.print("Time out error,\t"); 
		break;
    default: 
		Serial.print("Unknown error,\t"); 
		break;
  }
  // DISPLAY DATA
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.println(DHT.temperature, 1);
  datastreams[0].setFloat(DHT.temperature);
  datastreams[1].setFloat(DHT.humidity);
  Serial.println("Uploading it to Xively");
  int ret = xivelyclient.put(feed, xivelyKey);
  Serial.print("xivelyclient.put returned ");
  Serial.println(ret);

  Serial.println("Waiting 5min...");
  Serial.println();
  delay(5L * 60L * 1000L);
}
//
// END OF FILE
//
