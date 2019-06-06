/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This sketch shows how to write values to Virtual Pins

  NOTE:
  BlynkTimer provides SimpleTimer functionality:
    http://playground.arduino.cc/Code/SimpleTimer

  App project setup:
    Value Display widget attached to Virtual Pin V5
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SimpleTimer.h>
#include <SPI.h>
//#include <Ethernet.h>
//#include <BlynkSimpleEthernet.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "9b15e24a7d5e406ca7270cc23b35e91f";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "CASATINA";
char pass[] = "Minchiachefamiglia!3";
SimpleTimer timer;
const int trigPin = 2;
const int echoPin = 5;
const float AreaBott = 41.83265;
const float volBott = 836.653;
const int mlBott = 750;
long duration;
float volNow, mlNow, bevuti_live, bevutiOggi;
int distanceCm, distanceInch;
int lastData, lastMl;
String onOff;

//BlynkTimer timer;
WidgetRTC rtc;

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
BLYNK_CONNECTED(){
  bevutiOggi = 0;
  lastData = 0;
  lastMl = mlNow;
  Blynk.setProperty(V5, "min", 0);//Set the gauge min value
  Blynk.setProperty(V5, "max", 2000);//Set the gauge max value
  rtc.begin();
}
void checkDrinkedWater()
{
  
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  
  distanceCm= duration*0.034/2;
  volNow = AreaBott * distanceCm;
  bevuti_live= (volNow * mlBott)/ volBott;
  mlNow= mlBott - bevuti_live;
  Serial.println(distanceCm);
  int pos = onOff.indexOf("on"); 

 /*if(pos != -1){
     Serial.println("acceso");
  }*/

  if(lastData != bevuti_live && pos != -1 && distanceCm <= 22 && distanceCm >=3){
    Serial.println("acceso");
      if (lastData <= 749){
      bevutiOggi = lastData + (bevuti_live - lastData);
      lastData = bevutiOggi;
      lastMl = mlNow;
      onOff = "off";
      }else{
      bevutiOggi = lastData + bevuti_live;
      lastData = bevutiOggi;
      lastMl = mlNow;
      onOff = "off";
      }
      /*if(mlNow > lastMl){
      bevutiOggi = bevutiOggi - (mlNow - lastMl);
      lastData = bevutiOggi;
      }*/
    }
  Blynk.virtualWrite(V5, bevutiOggi);
}

// Digital clock display of the time
void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();
}

void setup()
{
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  // Debug console
  Serial.begin(9600);

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  timer.setInterval(1000L, checkDrinkedWater);
  setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)

  // Display digital clock every 10 seconds
  timer.setInterval(10000L, clockDisplay);
}

void loop(){

  Blynk.run();
  timer.run(); // Initiates BlynkTimer
  while(Serial.available()) {
  onOff= Serial.readString();// read the incoming data as string
}

}
