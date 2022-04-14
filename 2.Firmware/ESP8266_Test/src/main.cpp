/*
 * @Description: 
 * @Autor: Pi
 * @Date: 2022-04-14 12:48:12
 * @LastEditTime: 2022-04-14 15:55:12
 */
#include <Arduino.h>
#include <ESP8266WiFi.h>


#define LED1_PIN  14
#define LED2_PIN  12 

#define KEY1_PIN  13
#define KEY2_PIN  4
#define KEY3_PIN  5



const char* ssid     = "Moujiti";
const char* password = "moujiti7222";



void setup() {
  
  Serial.begin(115200);

  pinMode(LED1_PIN , OUTPUT);
  pinMode(LED2_PIN , OUTPUT);

  pinMode(KEY1_PIN , INPUT_PULLUP);
  pinMode(KEY2_PIN , INPUT_PULLUP);
  pinMode(KEY3_PIN , INPUT_PULLUP);

  digitalWrite(LED1_PIN , 1);
  digitalWrite(LED2_PIN , 1);



  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED2_PIN , 1);
    delay(250);
    Serial.print(".");
    digitalWrite(LED2_PIN , 0);
    delay(250);
  }

  digitalWrite(LED2_PIN , 0);

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

 

}

void loop() {
  
  Serial.print("WiFi RSSI:");
  Serial.println( WiFi.RSSI());

  delay(1000);

  int KEY1 = digitalRead(KEY1_PIN);
  int KEY2 = digitalRead(KEY2_PIN);
  int KEY3 = digitalRead(KEY3_PIN);

  Serial.print("KEY1:");
  Serial.print(KEY1);
  Serial.print("  KEY2:");
  Serial.print(KEY2);
  Serial.print("  KEY3:");
  Serial.println(KEY3);

 digitalWrite(LED2_PIN , 0);
  static bool LED_Status = 0;

  if(Serial.available()>0)
  {
    char ch=Serial.read();
    Serial.print(ch);
    digitalWrite(LED1_PIN , LED_Status);
    LED_Status = !LED_Status;
  }
}