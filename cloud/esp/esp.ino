#include <HTTPClient.h>
#include<WiFi.h>
#include <Arduino_JSON.h>

const char* serverName = "https://jsonplaceholder.typicode.com/todos/1";

void setup() {
 
    Serial.begin(9600);
    Serial.println("Hey!!!");
    WiFi.disconnect();
    delay(3000);
    Serial.println("START");
    WiFi.begin("Capable network", "dandelion2000");

      while((!(WiFi.status() == WL_CONNECTED))){
        delay(3000);
        Serial.print("..");
      }
      Serial.println("Connected");
      Serial.print("Your IP is: ");
      Serial.println((WiFi.localIP()));
}

void loop() {
  // put your main code here, to run repeatedly:
    HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(serverName);
   // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
    if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
    Serial.println(payload);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

 
}
