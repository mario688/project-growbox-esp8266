#include <ESP8266WiFi.h>
#include <WiFiManager.h>  
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <BH1750.h>
#include <FirebaseArduino.h>
#include "DHT.h"
#include "index.h"
#include "PCF8574.h"
#include "ccs811.h"
#define DHTPIN D4
#define DHTTYPE DHT22 
#define LEDPIN 13
#define trigPin D3
#define echoPin D0
#define FIREBASE_HOST "sturdy-dragon-299320-default-rtdb.firebaseio.com"              // the project name address from firebase id
#define FIREBASE_AUTH "1IUV4swwAL28S0DCaDbrLRiPThirJZ2Ow1tGmkDt"       // the secret key generated from firebase
const byte interruptPin = 12;
unsigned long previousTimeSensors = 0;
unsigned long previousTimePump= 0;
const String growbox_ID = "GROWBOX123";
long duration;
int distance;
DHT dht(DHTPIN, DHTTYPE);
PCF8574 pcf8574(0x20);
BH1750 lightMeter(0x23);
CCS811 ccs811;
WiFiManager wifiManager;
ESP8266WebServer server(80);
//elapsedMillis timeElapsed;
void ICACHE_RAM_ATTR resetDevice(){
  pcf8574.digitalWrite(P2, LOW);
  pcf8574.digitalWrite(P0, HIGH);
  WiFi.persistent(true);
  WiFi.disconnect(true);
  WiFi.persistent(false);
  delay(2000);  
  ESP.restart();
}
void handleRoot() 
{
 String s = webpage;
 server.send(200, "text/html", s);
}
void setup() {
  Serial.begin(9600);
  dht.begin();
  Wire.begin();
  ccs811.set_i2cdelay(50);
  pinMode(interruptPin,INPUT_PULLUP);
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT);
  pinMode(LEDPIN,OUTPUT);
  digitalWrite(LEDPIN, LOW);  
  attachInterrupt(digitalPinToInterrupt(interruptPin), resetDevice, RISING);
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("setup: CCS811 begin FAILED");
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("setup: CCS811 start FAILED");
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
  if (MDNS.begin("growboxpanel")) {
    Serial.println("DNS started: ");
    Serial.println("http://growboxpanel.local/");
  }
  server.on("/", handleRoot);
  server.on("/bright", brightness);
  server.on("/eco", eco);
  server.on("/etvoc", etvoc);
  server.on("/water", waterLevel);
  server.on("/soil", soliMoisture);
  server.on("/temp", temp);
  server.on("/hum", hum);
  server.on("/device_control", device_control);
  wifiManager.autoConnect("GrowBox");
  server.begin();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
void brightness(){
  if (lightMeter.measurementReady()) {
    int lux = lightMeter.readLightLevel();
    String sensor_brightValue = String(lux);
    Serial.print("Light: ");
    Serial.print(lux);
    Serial.println(" lx");
    sendToDB(String(lux),"lux");
    server.send(200, "text/plane", sensor_brightValue);
    
  }
}

void etvoc()
{
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 
  if( errstat==CCS811_ERRSTAT_OK ) {
    Serial.print("etvoc="); Serial.print(etvoc);    Serial.print(" ppb  ");
    server.send(200, "text/plane", String(etvoc));
    sendToDB(String(etvoc),"etvoc");
  }else{
    server.send(400, "text/plane", "CCS811 not working"); 
  }
}
void eco()
{ 
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 
  if( errstat==CCS811_ERRSTAT_OK ) { 
    Serial.print("eco2=");  Serial.print(eco2);     Serial.print(" ppm  ");
    server.send(200, "text/plane", String(eco2));
    sendToDB(String(eco2),"eco2");
   
  } else{ 
    server.send(404, "text/plane", "CCS811 not working"); 
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
   server.send(200, "text/plane", water_level);
  Serial.print("Water Level: ");
  Serial.println(water_level);
  sendToDB(String(water_level),"water");

 
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
    server.send(200, "text/plane", sensor_Soil);
    Serial.print("sensor_Soil: ");
    Serial.println(sensor_Soil);
    if(percentageHumididy<20)
    {
      
      pcf8574.digitalWrite(P4, HIGH);
      delay(10000);
      Serial.println("PODLEWANIE...");
      pcf8574.digitalWrite(P4, LOW);
      
    }
    sendToDB(String(sensor_Soil),"soil");
    
    
}
void hum()
{
   String hum = String(dht.readHumidity());
    Serial.print("Hum: ");
    Serial.print(hum);
    server.send(200, "text/plane", hum);
    sendToDB(String(hum),"hum");
}
void temp(){
  String temp = String(dht.readTemperature());
  Serial.print("Temp: ");
  Serial.print(temp);
  server.send(200, "text/plane", temp);
  sendToDB(String(temp),"temp");
 
}

void sendToDB(String value,String sensorName){
//{ Firebase.setString("devices/"+growbox_ID+"/sensors/"+sensorName, value);
//   if (Firebase.failed()) {
//      Serial.print("Can't set ");
//      Serial.println(sensorName);
//      Serial.println(Firebase.error());
//  }
}
void device_control()
{
  String act_state = server.arg("state");
  String act_fun = server.arg("fun");
  uint8_t state;
  uint8_t pin;
  if(act_fun=="pump")
  { pin=P4;
  }else if(act_fun=="lamp")
  { pin=P5;
  }else if(act_fun=="fan")
  { pin=P6;
  }
  if(act_state == "1")
  {
    state=HIGH;
  }else
  {
   state=LOW;
  }

  pcf8574.digitalWrite(pin, state);
  server.send(200, "text/plane", "done");
}
void loop() {
server.handleClient();
MDNS.update();
unsigned long currentTime = millis();
  if(WiFi.status() == WL_CONNECTED)
  {
    pcf8574.digitalWrite(P2, HIGH);
  }else
  { pcf8574.digitalWrite(P2, LOW);
    
  }
if(currentTime-previousTimeSensors >= 10000){
  brightness();
  hum();
  waterLevel();
  temp();
  etvoc();
  eco();
  soliMoisture();
  previousTimeSensors = currentTime;
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
