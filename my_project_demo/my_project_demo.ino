#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <DHT.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "ThingSpeak.h"

#define DHTPIN 5
#define DHTTYPE DHT22

#define LINE_TOKEN "ZrHx4oHAJMZyYuy9HJb3kxgsjXcB5ekgjdzXJsF0V61"

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "FARmer037"
#define AIO_KEY  "f7fc3c178a014d4bbada37a32acaf9cb"

WiFiClient client;

DHT dht(DHTPIN, DHTTYPE);

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temp");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish soilmoisture = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/soilmoisture");
Adafruit_MQTT_Publish lightintensity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lightintensity");
Adafruit_MQTT_Publish age = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/age");

//--------------------------------------------------------------------------------------------------------------------------------//
//const char* ssid = "SmartFarmNet";
//const char* password = "aptx4869";
const char* ssid = "AndroidAP";
const char* password = "fnei9721";

// Thingspeak channel details
unsigned long channelNumber = 963193;
const char * myWriteAPIKey = "UVLLKJC7997PHZTL";
const char * myReadAPIKey = "PJB25YZF2GE4Y4NC";
unsigned int tempFieldNumber = 1;
unsigned int humidFieldNumber = 2;
unsigned int soilFieldNumber = 3;
unsigned int lightFieldNumber = 4; 

int timezone = 7 * 3600;                      //ค่า TimeZone ตามเวลาประเทศไทย
int dst = 0;                                  //ค่า Date Swing Time

time_t plant = 1579172303;                    //ค่าเวลาปลูก (จำนวนวินาทีตั้งแต่ 1 มกราคม 1900 เวลา 00:00:00)

int soil_sensor = 34;
int led_wifi_status = 32;

const unsigned long eventIntervalTh = 420000;       // 7 นาที
const unsigned long eventIntervalAd = 660000;       // 11 นาที
unsigned long previousTimeTh = 0;
unsigned long previousTimeAd = 0;

//------------------------------------------------SETUP FUNCTION-------------------------------------------------------------------//
void setup() {
  pinMode(led_wifi_status, OUTPUT);
  digitalWrite(led_wifi_status, 0);
  
  Serial.begin(115200);
  dht.begin();
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nConnected to the WiFi network");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("");

  ThingSpeak.begin(client);  // Initialize ThingSpeak

}

//----------------------------------------------------------------------------------------------------------------------------------//

void loop() {
  if ((WiFi.status() == WL_CONNECTED))
  {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    int soil = read_soil();
    int ldr = read_ldr();
    int n_day = age_of_melon();

    unsigned long currentTime = millis();

    
    // ส่งทุก ๆ 7 นาที ส่งค่าไป thingspeak, ส่งทุก ๆ 11 นาที ส่งค่าไป Ardafruit
    if(currentTime - previousTimeTh >= eventIntervalTh) {
      print_value(t, h, soil, ldr, n_day);
      
      sendDataToThingspeak(t, h, soil);
      
      previousTimeTh = currentTime;
    }
    else if(currentTime - previousTimeAd >= eventIntervalAd) {
      print_value(t, h, soil, ldr, n_day);
      
      sendDataToAdafruit(temp, humidity, soilmoisture, lightintensity, age, t, h, soil, ldr, n_day);
      
      previousTimeAd = currentTime;
    }

    digitalWrite(led_wifi_status, 1);
  }
  else
  {
    Serial.println("Connection lost");
    digitalWrite(led_wifi_status, 0);
  }

  delay(1000);
}

//----------------------------------------------------------------------------------------------------------------------------------//

int read_soil() {
  int value1 = analogRead(soil_sensor);
  int soil = map(value1, 4095, 0, 0, 100);

  return soil;
}

int read_ldr() {
  int statusCode = 0;

  long light = ThingSpeak.readLongField(channelNumber, lightFieldNumber, myReadAPIKey);

  // Check the status of the read operation to see if it was successful
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    return light;
  }
  else{
    return 0;
  }
}

int age_of_melon() {
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);

  double diff = difftime(now, plant);
  int age = diff/86400;

  return age;
}

void print_value(int t, int h, int soil, int ldr, int age) {
  Serial.print("Temperature = ");
  Serial.print(t);
  Serial.print("      ");
  Serial.print("Humidity = ");
  Serial.print(h);
  Serial.println("");

  Serial.print("Soil Moistuer = ");
  Serial.print(soil);
  Serial.print("      ");
  Serial.print("Light Intensity = ");
  Serial.print(ldr);
  Serial.println("");

  Serial.print("Age of Melon = ");
  Serial.print(age);
  Serial.println(" Days");
}

boolean MQTT_connect() {  
  int8_t ret; 
  if (mqtt.connected()) {    
    return true; 
  }  
  uint8_t retries = 3;  
  while ((ret = mqtt.connect()) != 0) {
    mqtt.disconnect(); delay(2000);
    retries--;
    if (retries == 0) {
      return false; 
    }
  }
  return true;
}

void sendDataToAdafruit(Adafruit_MQTT_Publish feed_t, Adafruit_MQTT_Publish feed_h, Adafruit_MQTT_Publish feed_soil, Adafruit_MQTT_Publish feed_ldr, 
                        Adafruit_MQTT_Publish feed_n_day, int t, int h, int soil, int ldr, int n_day) {
  if (MQTT_connect()) {
    if(feed_t.publish(t) && feed_h.publish(h) && feed_soil.publish(soil) && feed_ldr.publish(ldr) && feed_n_day.publish(n_day)) {
      Serial.println("Data sent successfully.");
    }
    else {
      Serial.println("Problem to send the data!");
    }
  }
  else {
    Serial.println("Problem connect to the site!");
  }
}

void sendDataToThingspeak(int t, int h, int soil) {
//  ThingSpeak.writeField(channelNumber, tempFieldNumber, t, myWriteAPIKey);
//  ThingSpeak.writeField(channelNumber, humidFieldNumber, h, myWriteAPIKey);
//  ThingSpeak.writeField(channelNumber, soilFieldNumber, soil, myWriteAPIKey);

  // set the fields with the values
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, soil);

  ThingSpeak.writeFields(channelNumber, myWriteAPIKey);
}
