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
Adafruit_MQTT_Publish pumpswitch = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pumpswitch");
Adafruit_MQTT_Publish lightswitch = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/lightswitch");
Adafruit_MQTT_Publish age = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/age");

//------------------------------------------------MESSAGE--------------------------------------------------------------------------//
String m_Watered = "%E0%B8%A3%E0%B8%94%E0%B8%99%E0%B9%89%E0%B8%B3%E0%B9%81%E0%B8%A5%E0%B9%89%E0%B8%A7%20!";    //  รดน้ำแล้ว!
String m_TernOn = "%E0%B9%80%E0%B8%9B%E0%B8%B4%E0%B8%94%E0%B9%84%E0%B8%9F%20LED%20Grow%20Light%20%E0%B9%81%E0%B8%A5%E0%B9%89%E0%B8%A7%20!";   //  เปิดไฟ LED Grow Light แล้ว !
String m_TernOff = "%E0%B8%9B%E0%B8%B4%E0%B8%94%E0%B9%84%E0%B8%9F%20LED%20Grow%20Light%20%E0%B9%81%E0%B8%A5%E0%B9%89%E0%B8%A7%20!";           //  ปิดไฟ LED Grow Light แล้ว !

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

String ntp_day = "";
String ntp_time = "";

time_t plant = 1579172303;                    //ค่าเวลาปลูก (จำนวนวินาทีตั้งแต่ 1 มกราคม 1900 เวลา 00:00:00)

//-----------------------SET PIN-----------------------------------------//
int pump = 32;
int led = 33;
int soil_sensor = 34;
int led_wifi_status = 32;

//------------------------------------------------SET STATE------------------------------------------------------------------------//
int state_water = 0;
int state_light = 0;
int state_day = 100;

const unsigned long eventIntervalTh = 420000;       // 7 นาที
const unsigned long eventIntervalAd = 660000;       // 11 นาที
unsigned long previousTimeTh = 0;
unsigned long previousTimeAd = 0;

//------------------------------------------------SETUP FUNCTION-------------------------------------------------------------------//
void setup() {
  pinMode(pump, OUTPUT);
  pinMode(led, OUTPUT);
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

    water(soil);
    turnOnTheLight(ldr);

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

String currentTime() {
  String current_t = "";
  
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);

  ntp_time = String(p_tm->tm_hour);
  ntp_time += ":";
  ntp_time += String(p_tm->tm_min);
  ntp_time += ":";
  ntp_time += String(p_tm->tm_sec);
  
  ntp_day = String(p_tm->tm_mday); 
  ntp_day += "-";
  ntp_day += String(p_tm->tm_mon + 1);
  ntp_day += "-";  
  ntp_day += String(p_tm->tm_year + 1900);

  current_t = ntp_day + " " + ntp_time;

  return current_t;
}

void water(int soil) {
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  
  String current_t = currentTime();

  if(soil < 80) {
    if(p_tm->tm_hour == 8 && p_tm->tm_min == 0 && p_tm->tm_sec == 0) {
      digitalWrite(pump, 1);
      sendStatusToAdafruit(pumpswitch, "ON");
      
      if(soil >= 80 || p_tm->tm_sec == 20) {
        digitalWrite(pump, 0);
        LINE_Notify("\n" + current_t + "\n" + m_Watered);
        sendStatusToAdafruit(pumpswitch, "OFF");
      }
    }
    else if(p_tm->tm_hour == 11 && p_tm->tm_min == 0 && p_tm->tm_sec == 0) {
      digitalWrite(pump, 1);
      sendStatusToAdafruit(pumpswitch, "ON");
      
      if(soil >= 80 || p_tm->tm_sec == 20) {
        digitalWrite(pump, 0);
        LINE_Notify("\n" + current_t + "\n" + m_Watered);
        sendStatusToAdafruit(pumpswitch, "OFF");
      }
    }
    else if(p_tm->tm_hour == 14 && p_tm->tm_min == 0 && p_tm->tm_sec == 0) {
      digitalWrite(pump, 1);
      sendStatusToAdafruit(pumpswitch, "ON");
      
      if(soil >= 80 || p_tm->tm_sec == 20) {
        digitalWrite(pump, 0);
        LINE_Notify("\n" + current_t + "\n" + m_Watered);
        sendStatusToAdafruit(pumpswitch, "OFF");
      }
    }
  }
}

void turnOnTheLight(int ldr) {
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);

  String current_t = currentTime();

  if((p_tm->tm_hour >= 18) || p_tm->tm_hour <= 6 || ldr < 50) {
    digitalWrite(led, 1);

    if(state_light == 0) {
      LINE_Notify("\n" + current_t + "\n" + m_TernOn);
      sendStatusToAdafruit(lightswitch, "ON");
      state_light = 1;
    }
  }
  else {
    digitalWrite(led, 0);

    if(state_light == 1) {
      LINE_Notify("\n" + current_t + "\n" + m_TernOff);
      sendStatusToAdafruit(lightswitch, "OFF");
      state_light = 0;
    }
  }
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

void sendStatusToAdafruit(Adafruit_MQTT_Publish feed, const char* sw_status) {
  if (MQTT_connect()) {
    if(feed.publish(sw_status)) {
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
  // set the fields with the values
  ThingSpeak.setField(1, t);
  ThingSpeak.setField(2, h);
  ThingSpeak.setField(3, soil);

  ThingSpeak.writeFields(channelNumber, myWriteAPIKey);
}

bool LINE_Notify(String message) {
  WiFiClientSecure client;

  if (!client.connect("notify-api.line.me", 443)) {
    Serial.println("connection failed");
    return false;   
  }

  String payload = "message=" + message;
  String req = "";
  req += "POST /api/notify HTTP/1.1\r\n";
  req += "Host: notify-api.line.me\r\n";
  req += "Authorization: Bearer " + String(LINE_TOKEN) + "\r\n";
  req += "User-Agent: ESP32\r\n";
  req += "Content-Type: application/x-www-form-urlencoded\r\n";
  req += "Content-Length: " + String(payload.length()) + "\r\n";
  req += "\r\n";
  req += payload;
  // Serial.println(req);
  client.print(req);
    
  delay(20);

  // Serial.println("-------------");
  long timeOut = millis() + 30000;
  while(client.connected() && timeOut > millis()) {
    if (client.available()) {
      String str = client.readString();
      // Serial.print(str);
    }
    delay(10);
  }
  // Serial.println("-------------");

  return timeOut > millis();
}
