#include <WiFi.h>
#include <time.h>

//--------------------------------------------------------------------------------------------------------------------------------//
//const char* ssid = "SmartFarmNet";
//const char* password = "aptx4869";
const char* ssid = "AndroidAP";
const char* password = "fnei9721";

int timezone = 7 * 3600;                      //ค่า TimeZone ตามเวลาประเทศไทย
int dst = 0;                                  //ค่า Date Swing Time

time_t plant = 1579172303;

String ntp_day = "";
String ntp_time = "";

int pump_status = 0;

//------------------------------------------------SETUP FUNCTION-------------------------------------------------------------------//
void setup() {
  
  Serial.begin(115200);
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

}

//----------------------------------------------------------------------------------------------------------------------------------//

void loop() {
  if ((WiFi.status() == WL_CONNECTED))
  {
    int n_day = age_of_melon();
    String current_t = currentTime();

    water(n_day);
    
  }
  else
  {
    Serial.println("Connection lost");
  }

  delay(3000);
}

int age_of_melon() {
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);

  double diff = difftime(now, plant);
  int age = diff/86400;

//  Serial.print("Age = ");
//  Serial.println(age);

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

void water(int age) {
  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  
  String current_t = currentTime();

  if (age <= 14) {
    if (p_tm->tm_hour == 8) {
      if (p_tm->tm_min == 0 && p_tm->tm_sec <= 20) {
        Serial.println("Pump ON!");
      
        /* 
         * digitalWrite(relay_pump, 1);
         * if (pump_status == 0) {
         * sendStatusToAdafruit(pumpswitch, "ON");
         * }
         */
         
        pump_status = 1;
      }
      else {
        Serial.println("Pump OFF!");   
          
        // digitalWrite(relay_pump, 0);
        
        if (pump_status == 1) {
          Serial.print(current_t);       
          Serial.println(" Watered");
          
          /*
           * LINE_Notify("\n" + current_t + "\n" + m_Watered);
           * sendStatusToAdafruit(pumpswitch, "OFF");
           */
           
          pump_status = 0;
        }
      }
    }
    else if (p_tm->tm_hour == 15) {
      if (p_tm->tm_min == 0 && p_tm->tm_sec <= 20) {
        Serial.println("Pump ON!");
        
        /* 
         * digitalWrite(relay_pump, 1);
         * if (pump_status == 0) {
         * sendStatusToAdafruit(pumpswitch, "ON");
         * }
         */
         
        pump_status = 1;
      }
      else {
        Serial.println("Pump OFF!");   
          
        // digitalWrite(relay_pump, 0);
        
        if (pump_status == 1) {
          Serial.print(current_t);       
          Serial.println(" Watered");
          
          /*
           * LINE_Notify("\n" + current_t + "\n" + m_Watered);
           * sendStatusToAdafruit(pumpswitch, "OFF");
           */
           
          pump_status = 0;
        }
      }
    }
  }
  else if (age > 14) {
    if (p_tm->tm_hour == 8) {
      if (p_tm->tm_min == 0 && p_tm->tm_sec <= 20) {
        Serial.println("Pump ON!");
      
        /* 
         * digitalWrite(relay_pump, 1);
         * if (pump_status == 0) {
         * sendStatusToAdafruit(pumpswitch, "ON");
         * }
         */
         
        pump_status = 1;
      }
      else {
        Serial.println("Pump OFF!");   
          
        // digitalWrite(relay_pump, 0);
        
        if (pump_status == 1) {
          Serial.print(current_t);       
          Serial.println(" Watered");
          
          /*
           * LINE_Notify("\n" + current_t + "\n" + m_Watered);
           * sendStatusToAdafruit(pumpswitch, "OFF");
           */
           
          pump_status = 0;
        }
      }
    }
    else if (p_tm->tm_hour == 12) {
      if (p_tm->tm_min == 0 && p_tm->tm_sec <= 20) {
        Serial.println("Pump ON!");
      
        /* 
         * digitalWrite(relay_pump, 1);
         * if (pump_status == 0) {
         * sendStatusToAdafruit(pumpswitch, "ON");
         * }
         */
         
        pump_status = 1;
      }
      else {
        Serial.println("Pump OFF!");   
          
        // digitalWrite(relay_pump, 0);
        
        if (pump_status == 1) {
          Serial.print(current_t);       
          Serial.println(" Watered");
          
          /*
           * LINE_Notify("\n" + current_t + "\n" + m_Watered);
           * sendStatusToAdafruit(pumpswitch, "OFF");
           */
           
          pump_status = 0;
        }
      }
    }
    else if (p_tm->tm_hour == 15) {
      if (p_tm->tm_min == 0 && p_tm->tm_sec <= 20) {
        Serial.println("Pump ON!");
        
        /* 
         * digitalWrite(relay_pump, 1);
         * if (pump_status == 0) {
         * sendStatusToAdafruit(pumpswitch, "ON");
         * }
         */
         
        pump_status = 1;
      }
      else {
        Serial.println("Pump OFF!");   
          
        // digitalWrite(relay_pump, 0);
        
        if (pump_status == 1) {
          Serial.print(current_t);       
          Serial.println(" Watered");
          
          /*
           * LINE_Notify("\n" + current_t + "\n" + m_Watered);
           * sendStatusToAdafruit(pumpswitch, "OFF");
           */
           
          pump_status = 0;
        }
      }
    }
  }
}
