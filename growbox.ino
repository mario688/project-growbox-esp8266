#include <ESP8266WiFi.h>
#include <WiFiManager.h>  
#include <Wire.h>
#include <BH1750.h>
#include <FirebaseArduino.h>
#include "DHT.h"
#include "PCF8574.h"


#define DHTPIN D4
#define DHTTYPE DHT22 
#define LEDPIN 13
#define trigPin D3
#define echoPin D0
const byte interruptPin = 12;
unsigned long previousTimeWater = 0;
unsigned long previousTimeDHT22 = 0;
const String growbox_ID = "GROWBOX123";
long duration;
int distance;
DHT dht(DHTPIN, DHTTYPE);
PCF8574 pcf8574(0x20);
BH1750 lightMeter(0x23);
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
   Wire.begin();
   pinMode(interruptPin,INPUT_PULLUP);
   pinMode(trigPin, OUTPUT); 
   pinMode(echoPin, INPUT);
   pinMode(LEDPIN,OUTPUT);
   digitalWrite(LEDPIN, LOW);  
   attachInterrupt(digitalPinToInterrupt(interruptPin), resetDevice, RISING);
  for(int i=0;i<8;i++)
  {
    pcf8574.pinMode(i, OUTPUT,LOW);
  }
  if (lightMeter.begin()) {
    Serial.println(F("BH1750 ok"));
  }
  else {
    Serial.println(F("Error initialising BH1750"));
  }
  Serial.print("Init pcf8574...");
  if (pcf8574.begin()){
    Serial.println("OK");
  }else{
    Serial.println("KO");
  }
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
void brightness(){
  if (lightMeter.measurementReady()) {
    float lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
  }
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
  Serial.print("Water Level: ");
  Serial.println(water_level);
//Firebase.setString("devices/"+growbox_ID+"/sensors/water", water_level);
//  if (Firebase.failed()) {
//      Serial.print("Can't set Humidity");
//      Serial.println(Firebase.error());
//  }
 
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
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("  ");
  Serial.print("Hum: ");
  Serial.print(hum);
//  Firebase.setFloat("devices/"+growbox_ID+"/sensors/temp", temp);
//  if (Firebase.failed()) {
//      Serial.print("Can't set Humidity");
//      Serial.println(Firebase.error());
//  }
//  Firebase.setFloat("devices/"+growbox_ID+"/sensors/hum", hum);
//   if (Firebase.failed()) {
//      Serial.print("Can't set Humidity");
//      Serial.println(Firebase.error());
//  }
}



void loop() {
unsigned long currentTime = millis();
  pcf8574.digitalWrite(P0, HIGH);
  delay(200);
  pcf8574.digitalWrite(P0, LOW);
  delay(200);
//
if(currentTime-previousTimeWater >= 5000){
  waterLevel();
  brightness();
  getValueFromSensors();
  previousTimeWater = currentTime;
}
//if(currentTime-previousTimeDHT22 >= 60000){
//  soliMoisture();
//    getValueFromSensors();
//  previousTimeDHT22 = currentTime;
//}


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
