
/*  Grzegorz Pawlak
    grzes.pawlak@gmail.com
    Irrigation System
    based on ESP32 and Blynk 2.0 platform
    Firmware werson 0.1.3
*/

// 0.1.1 - full WIFI functionality, with general OFF button
// 0.1.2 - added rain detector and implemented into general OFF button 
// 0.1.3 - rebulding system  
// 0.1.4 - updated time and run times

#define BLYNK_TEMPLATE_ID "TMPLcq7dN-Vf"
#define BLYNK_DEVICE_NAME "Irrigation"

#define BLYNK_FIRMWARE_VERSION        "0.1.4"

#define BLYNK_PRINT Serial

#define APP_DEBUG

#define USE_WROVER_BOARD

#include "BlynkEdgent.h"
#include <Wire.h>
#include "ds3231.h"

struct ts t;  // for RTC

const int rainSensor = 23;  // ESP32's pin number for rain sensor
int rainSensorState = 0;  // 0 means no rain detected

const int valve1 = 27;  // defining ESP32's valve port
const int valve2 = 14;
const int valve3 = 32;
const int valve4 = 33;
const int valve5 = 25;  
const int valve6 = 26;

int offCount = 0;  // variable for the main turn-off switch
bool offButton = 0;  // the state of the turn-off switch
int lastOffHour = 0;

// settings for 1st valve:
int startH1 = 0;  // start hour
int startM1 = 0;  // start minute
int runTime1 = 20;  // run time in minutes
int count1 = 0;
int lastMin1 = 0;
bool button1 = 0;
int virtualPin1 = 1;

// settings for 2st valve:
int startH2 = 1;  // start hour
int startM2 = 0;  // start minute
int runTime2 = 20;  // run time in minutes
int count2 = 0;
int lastMin2 = 0;
bool button2 = 0;
int virtualPin2 = 2;

// settings for 3st valve:
int startH3 = 2;  // start hour
int startM3 = 0;  // start minute
int runTime3 = 20;  // run time in minutes
int count3 = 0;
int lastMin3 = 0;
bool button3 = 0;
int virtualPin3 = 3;

// settings for 4st valve:
int startH4 = 3;  // start hour
int startM4 = 0;  // start minute
int runTime4 = 20;  // run time in minutes
int count4 = 0;
int lastMin4 = 0;
bool button4 = 0;
int virtualPin4 = 4;

// settings for 5st valve:
int startH5 = 4;  // start hour
int startM5 = 0;  // start minute
int runTime5 = 20;  // run time in minutes
int count5 = 0;
int lastMin5 = 0;
bool button5 = 0;
int virtualPin5 = 5;

// settings for 6st valve:
int startH6 = 5;  // start hour
int startM6 = 0;  // start minute
int runTime6 = 20;  // run time in minutes
int count6 = 0;
int lastMin6 = 0;
bool button6 = 0;
int virtualPin6 = 6;

BLYNK_WRITE(V0) {
  int pinValue = param.asInt();  // assigning incoming value from pin Vx to a variable, x is virtual pin number
  if (pinValue == 1) { // If the button is ON, the offCount starts, 
    offButton = 1;  // the button bool variable == 1
    offCount = 1;  // the clock starts
    lastOffHour = t.hour;
  } else {
    offButton = 0;
    offCount = 0;
  }
}

BLYNK_WRITE(V1) {
  int pinValue = param.asInt();  // assigning incoming value from pin Vx to a variable, x is virtual dpin number
  digitalWrite(valve1, !pinValue);
  if (pinValue == 1) { // If the button is ON, the value of boolCount turns into 1 and opposite
    button1 = 1;  // additional bool variable to
  } else {
    button1 = 0;
  }
}

BLYNK_WRITE(V2) {
  int pinValue = param.asInt();  // assigning incoming value from pin Vx to a variable, x is virtual dpin number
  digitalWrite(valve2, !pinValue);
  if (pinValue == 1) { // If the button is ON, the value of boolCount turns into 1 and opposite
    button2 = 1;  // additional bool variable to
  } else {
    button2 = 0;
  }
}

BLYNK_WRITE(V3) {
  int pinValue = param.asInt();  // assigning incoming value from pin Vx to a variable, x is virtual dpin number
  digitalWrite(valve3, !pinValue);
  if (pinValue == 1) { // If the button is ON, the value of boolCount turns into 1 and opposite
    button3 = 1;  // additional bool variable to
  } else {
    button3 = 0;
  }
}

BLYNK_WRITE(V4) {
  int pinValue = param.asInt();  // assigning incoming value from pin Vx to a variable, x is virtual dpin number
  digitalWrite(valve4, !pinValue);
  if (pinValue == 1) { // If the button is ON, the value of boolCount turns into 1 and opposite
    button4 = 1;  // additional bool variable to
  } else {
    button4 = 0;
  }
}

BLYNK_WRITE(V5) {
  int pinValue = param.asInt();  // assigning incoming value from pin Vx to a variable, x is virtual dpin number
  digitalWrite(valve5, !pinValue);
  if (pinValue == 1) { // If the button is ON, the value of boolCount turns into 1 and opposite
    button5 = 1;  // additional bool variable to
  } else {
    button5 = 0;
  }
}

BLYNK_WRITE(V6) {
  int pinValue = param.asInt();  // assigning incoming value from pin Vx to a variable, x is virtual dpin number
  digitalWrite(valve6, !pinValue);
  if (pinValue == 1) { // If the button is ON, the value of boolCount turns into 1 and opposite
    button6 = 1;  // additional bool variable to
  } else {
    button6 = 0;
  }
}

void runRequirements(int valveNr, int runTime, int &count, int &lastMin, int virtualPin, bool &button, int startH, int startM) { // starting requirements
  if ( ( ( (startH == t.hour) && (startM == t.min) && (button == 0) && (count == 0) ) || ( (button == 1) && (count == 0) ) ) && (offButton == 0) ) { // if reached the opening time or it was manual opening
    lastMin = t.min;  // saving the actual minute
    count = 1;  // starting the count
    if (button == 0) { // this is necessery for scheduled run (in that case we start with button = 0
      digitalWrite(valveNr, 0); // opening the valve, 0 is open
      Blynk.virtualWrite(virtualPin, 1);
      button = 1;
    }
  }
  if ( (button == 0) && (count != 0) && (t.min != lastMin) ) { // the valve changed it's state to closed, if the actual minute has already change (because otherwise it would fullfil the requirement of the firt IF in the runRequirements function
    count = 0;
  } else {
    return;
  }
}

void stopWatch(int valveNr, int runTime, int &count, int &lastMin, int virtualPin, bool &button) {
  if ( (count != 0) && (count <= runTime) && (lastMin != t.min) ) { // if the minute count is still lower than runTime (in minutes) and the present minute has changed
    count += 1;  // adding another minute to count
    lastMin = t.min;  // saving actual minute to variable
  }
  if ( ( (count != 0) && (count > runTime) ) || (offButton == 1) ) { // if the count is greater than runTime, the valve is closing
    digitalWrite(valveNr, 1);  // closing valve
    Blynk.virtualWrite(virtualPin, 0);  // turning button off
    button = 0;
    count = 0;
  }
  
  // condidtions for turn-off button
  if ( (offButton == 1) && (offCount < 24) && (lastOffHour != t.hour) ){
    offCount += 1;
  } 
  if( (offCount == 24) ){  // after 24h 
    Blynk.virtualWrite(0, 0);
    offButton = 0;
    offCount = 0;
  } else {
    return;
  }
}

void setup() {
  pinMode(valve1, OUTPUT);
  digitalWrite(valve1, 1);
  pinMode(valve2, OUTPUT);
  digitalWrite(valve2, 1);
  pinMode(valve3, OUTPUT);
  digitalWrite(valve3, 1);
  pinMode(valve4, OUTPUT);
  digitalWrite(valve4, 1);
  pinMode(valve5, OUTPUT);
  digitalWrite(valve5, 1);
  pinMode(valve6, OUTPUT);
  digitalWrite(valve6, 1);

  pinMode(rainSensor, INPUT);
  pinMode(2, OUTPUT);

  Serial.begin(9600);
  Wire.begin();
  DS3231_init(DS3231_CONTROL_INTCN);
  delay(100);

  t.hour = 16;  // initial clock setting
  t.min = 30;
  t.sec = 30;
  t.mday = 24;
  t.mon = 4;
  t.year = 2023;
  
  DS3231_set(t);

  BlynkEdgent.begin();
}

void loop() {
  DS3231_get(&t);
  BlynkEdgent.run();

  rainSensorState = digitalRead(rainSensor);
  if(rainSensorState == 1){  // no rain
    digitalWrite(2, 1);
  }else{  // rain
    digitalWrite(2, 0);
    Blynk.virtualWrite(0, 1);  // putting on the off button
    offButton = 1;  // the button bool variable == 1
    offCount = 1;  // the clock starts
    lastOffHour = t.hour;
  }

  // main functions
  runRequirements(valve1, runTime1, count1, lastMin1, virtualPin1, button1, startH1, startM1);
  stopWatch(valve1, runTime1, count1, lastMin1, virtualPin1, button1);

  runRequirements(valve2, runTime2, count2, lastMin2, virtualPin2, button2, startH2, startM2);
  stopWatch(valve2, runTime2, count2, lastMin2, virtualPin2, button2);

  runRequirements(valve3, runTime3, count3, lastMin3, virtualPin3, button3, startH3, startM3);
  stopWatch(valve3, runTime3, count3, lastMin3, virtualPin3, button3);

  runRequirements(valve4, runTime4, count4, lastMin4, virtualPin4, button4, startH4, startM4);
  stopWatch(valve4, runTime4, count4, lastMin4, virtualPin4, button4);

  runRequirements(valve5, runTime5, count5, lastMin5, virtualPin5, button5, startH5, startM5);
  stopWatch(valve5, runTime5, count5, lastMin5, virtualPin5, button5);

  runRequirements(valve6, runTime6, count6, lastMin6, virtualPin6, button6, startH6, startM6);
  stopWatch(valve6, runTime6, count6, lastMin6, virtualPin6, button6);


  Serial.print("\t Hour : ");
  Serial.print(t.hour);
  Serial.print(":");
  Serial.print(t.min);
  Serial.print(".");
  Serial.println(t.sec);
}
