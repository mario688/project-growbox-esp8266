#include <ESP8266WiFi.h>
#include <WiFiManager.h>  
#include <FirebaseArduino.h>
#include "DHT.h"



#define DHTPIN D2
const byte interruptPin = 12;
#define DHTTYPE DHT22 
#define LEDPIN 13
#define trigPin D3
#define echoPin D0
unsigned long previousTimeWater = 0;
unsigned long previousTimeDHT22 = 0;
const String growbox_ID = "GROWBOX123";
long duration;
int distance;
DHT dht(DHTPIN, DHTTYPE);
WiFiManager wifiManager;
//elapsedMillis timeElapsed;
void ICACHE_RAM_ATTR resetDevice(){
   digitalWrite(LEDPIN, HIGH);
   delay(1000);   
   wifiManager.resetSettings();
   ESP.reset();
}
void setup() {
   Serial.begin(9600);
   dht.begin();
   pinMode(interruptPin,INPUT_PULLUP);
   pinMode(trigPin, OUTPUT); 
   pinMode(echoPin, INPUT);
   pinMode(LEDPIN,OUTPUT);
   digitalWrite(LEDPIN, LOW);  
   attachInterrupt(digitalPinToInterrupt(interruptPin), resetDevice, RISING);
  
  
  wifiManager.autoConnect("GrowBox");
//  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
//  Serial.print("connecting");
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print(".");
//    delay(500);
//  }
//  Serial.println();
//  Serial.print("connected: ");
//  Serial.println(WiFi.localIP());
  
 Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
 
}

void waterLevel()
{
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance= duration*0.034/2;
  int percentageLvlWater = map(distance, 11,3,0,100);
  if(percentageLvlWater>100)
  {
    percentageLvlWater = 100;
  }else if(percentageLvlWater<0)
  {
    percentageLvlWater = 0;
  }
  String water_level = String(percentageLvlWater);
  Firebase.setString("devices/"+growbox_ID+"/sensors/water", water_level);
  if (Firebase.failed()) {
      Serial.print("Can't set Humidity");
      Serial.println(Firebase.error());
  }
 
}
void soliMoisture(){
    const int AirValue = 620;
    const int WaterValue = 310;
    int soiVal = analogRead(A0);
    int percentageHumididy = map(soiVal, AirValue,WaterValue,0,100);
    if(percentageHumididy>=100){
      percentageHumididy=100; 
    }else if(percentageHumididy<=0)
      percentageHumididy=0; 
    String sensor_Soil = String(percentageHumididy);
     Firebase.setString("devices/"+growbox_ID+"/sensors/soil", sensor_Soil);
     if (Firebase.failed()) {
      Serial.print("Can't set Humidity");
      Serial.println(Firebase.error());
     }
    
}
void getValueFromSensors(){
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  Firebase.setFloat("devices/"+growbox_ID+"/sensors/temp", temp);
  if (Firebase.failed()) {
      Serial.print("Can't set Humidity");
      Serial.println(Firebase.error());
  }
  Firebase.setFloat("devices/"+growbox_ID+"/sensors/hum", hum);
   if (Firebase.failed()) {
      Serial.print("Can't set Humidity");
      Serial.println(Firebase.error());
  }
}



void loop() {
unsigned long currentTime = millis();


if(currentTime-previousTimeWater >= 60000){
  waterLevel();
  previousTimeWater = currentTime;
}
if(currentTime-previousTimeDHT22 >= 60000){
  soliMoisture();
    getValueFromSensors();
  previousTimeDHT22 = currentTime;
}


//getValueFromSensors();
  
 
    

  // get value 
//  Serial.print("number: ");
//  Serial.println(Firebase.getFloat("humidity"));
//  delay(1000);

  // remove value
  //Firebase.remove("temperature");
  //delay(1000);

  // set string value
  //Firebase.setString("message", "hello world");
  // handle error
//  if (Firebase.failed()) {
//      Serial.print("setting /message failed:");
//      Serial.println(Firebase.error());  
//      return;
//  }
  //delay(1000);
  
  // set bool value
 // Firebase.setBool("truth", false);
  // handle error
//  if (Firebase.failed()) {
//      Serial.print("setting /truth failed:");
//      Serial.println(Firebase.error());  
//      return;
//  }
//  delay(1000);

  // append a new value to /logs
//  String name = Firebase.pushInt("logs", n++);
//  // handle error
//  if (Firebase.failed()) {
//      Serial.print("pushing /logs failed:");
//      Serial.println(Firebase.error());  
//      return;
//  }
//  Serial.print("pushed: /logs/");
//  Serial.println(name);
//  delay(1000);
}
