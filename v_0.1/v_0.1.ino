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
//Gyro
#include<Wire.h>
const int MPU_addr=0x68; // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "9b15e24a7d5e406ca7270cc23b35e91f";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "LAZIO_INNOVA_GUEST"; // CASATINA - wifi-gratis - LAZIO_INNOVA_GUEST
char pass[] = "Li-Psw-16";//Minchiachefamiglia!3 - culocane97 - Li-Psw-16
SimpleTimer timer;
const int trigPin = 2;
const int echoPin = 5;
const float AreaBott = 41.83265;
const float volBott = 836.653;
const int mlBott = 750;
long duration;
float volNow, mlNow, bevuti_live, bevutiOggi;
int distanceCm, distanceInch;
float lastData, lastMl, lastDrinkedMl;
String onOff;
float scarto = 0;
bool riempito,dritta;

//BlynkTimer timer;
WidgetRTC rtc;

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
BLYNK_CONNECTED(){
  bevutiOggi = 0;
  lastData = 0;
  lastMl = mlNow;
  scarto = 0;
  Blynk.setProperty(V5, "min", 0);//Set the gauge min value
  Blynk.setProperty(V5, "max", 2000);//Set the gauge max value
  rtc.begin();
  Blynk.virtualWrite(V5, bevutiOggi);
}

void checkStationary(){
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true); // request a total of 14 registers

  

  AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  dritta=false;
  if(AcX>=450 && AcX<=550 && AcY>=50 && AcY<=150 && AcZ>=15900 && AcZ<=16100 ){
    Serial.println("la bott. è dritta");
    dritta = !dritta;
  }

  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53); //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
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
  //Serial.println(pos);
  Serial.print("b_live: ");
  Serial.println(bevuti_live);
  Serial.print("ultimi bevuti: ");
  Serial.println(lastDrinkedMl);

  /*if(mlNow > lastMl && mlNow > 0){
    scarto = mlNow - lastMl;
    Serial.print("scarto: ");
    Serial.println(scarto);
  }*/
  riempito = false;
  if(lastDrinkedMl != bevuti_live /*&& pos != -1*/ && distanceCm <= 22 && distanceCm >=2 && dritta){
    Serial.println("entrato");
   /* Serial.println(riempito);
      if (lastData <= 749){
      bevutiOggi = lastData + (bevuti_live - lastData);
      lastData = bevutiOggi;
      lastMl = mlNow;
      onOff = "off";
      }else{*/
      if(mlNow > lastMl){
        riempito = true;
        //scarto = mlNow - lastMl;
        lastMl = mlNow;
        lastDrinkedMl = 0;
      }
     // Serial.println(riempito);
      if(!riempito){
        bevutiOggi = lastData + (bevuti_live - lastDrinkedMl); //(bevuti_live - scarto);
        lastData = bevutiOggi;
        lastDrinkedMl = bevuti_live;
        lastMl = mlNow;
        scarto = 0;
        Blynk.virtualWrite(V5, bevutiOggi);
      }
      onOff = "off";
    }

  
}

// Digital clock display of the time
void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
 /* Serial.print("Current time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();*/
}

void setup()
{
  //Gyro
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  
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
  timer.setInterval(1000L, checkStationary);

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
