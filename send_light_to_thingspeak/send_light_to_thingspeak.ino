#include "ThingSpeak.h"
#include "secrets.h"
#include <WiFi.h>

char ssid[] = SECRET_SSID;   // your network SSID (name) 
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key Index number (needed only for WEP)
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
unsigned int fieldNumber = 4;

int ldr_sensor = 35;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);   

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  
}

void loop() {
  int light = read_ldr();

  // Write to ThingSpeak.
  int x = ThingSpeak.writeField(myChannelNumber, fieldNumber, light, myWriteAPIKey);
  if(x == 200){
    Serial.print("Channel update successful. Light = ");
    Serial.println(light);
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  delay(420000);

}

int read_ldr() {
  int value2 = analogRead(ldr_sensor);
  int ldr = map(value2, 4095, 0, 0, 100);

  return ldr;
}
