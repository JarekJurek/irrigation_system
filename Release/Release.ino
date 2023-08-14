
/*  Grzegorz Pawlak
    grzes.pawlak@gmail.com
    Irrigation System
    based on ESP32 and Blynk 2.0 platform
    Firmware werson 0.1.5
*/

// 0.1.1 - full WIFI functionality, with general OFF button
// 0.1.2 - added rain detector and implemented into general OFF button 
// 0.1.3 - rebulding system  
// 0.1.4 - updated time and run times
// 0.1.5 - updated time and run times

#define BLYNK_TEMPLATE_ID "TMPLcq7dN-Vf"
#define BLYNK_DEVICE_NAME "Irrigation"

#define BLYNK_FIRMWARE_VERSION        "0.1.5"

#define BLYNK_PRINT Serial

#define APP_DEBUG

#define USE_WROVER_BOARD

#include "BlynkEdgent.h"
#include <Wire.h>
#include "ds3231.h"

struct ts t;  // for RTC

const int rainSensorPin = 23;   // ESP32's pin number for rain sensor
bool rainDetected = 0;

const int valve1 = 27;  // defining ESP32's valve port
const int valve2 = 14;
const int valve3 = 32;
const int valve4 = 33;
const int valve5 = 25;  
const int valve6 = 26;

int offCount = 0;   // variable for the main turn-off switch
bool offButtonOn = false;   // the state of the turn-off switch
int lastOffHour = 0;

int startH1 = 0;    // start hour for vale 1
int startM1 = 0;    // start minute
int runTime1 = 20;  // run time in minutes
int count1 = 0;     // couter for minutes with open valve
int lastMin1 = 0;   // for catching that minute has changed
bool valveButton1 = 0;  // inital state of valve's button
int virtualPin1 = 1;

int startH2 = 1;
int startM2 = 0;
int runTime2 = 20;
int count2 = 0;
int lastMin2 = 0;
bool valveButton2 = 0;
int virtualPin2 = 2;

int startH3 = 2;
int startM3 = 0;
int runTime3 = 20;
int count3 = 0;
int lastMin3 = 0;
bool valveButton3 = 0;
int virtualPin3 = 3;

int startH4 = 3;
int startM4 = 0;
int runTime4 = 20;
int count4 = 0;
int lastMin4 = 0;
bool valveButton4 = 0;
int virtualPin4 = 4;

int startH5 = 4;
int startM5 = 0;
int runTime5 = 20;
int count5 = 0;
int lastMin5 = 0;
bool valveButton5 = 0;
int virtualPin5 = 5;

int startH6 = 5;
int startM6 = 0;
int runTime6 = 20;
int count6 = 0;
int lastMin6 = 0;
bool valveButton6 = 0;
int virtualPin6 = 6;

BLYNK_WRITE(V0) {                 // Main turn-off button
  int valueHigh = param.asInt();  // assigning incoming value from pin Vx to a variable, x is virtual pin number
  if (valueHigh) {                // If the button is ON, the offCount starts, 
    offButtonOn = true;           // storing the state of button 
    offCount = 1;                 // the offButton's clock starts
    lastOffHour = t.hour;
  } else {
    offButtonOn = false;
    offCount = 0;
  }
}

BLYNK_WRITE(V1) {
  int valueHigh = param.asInt();
  digitalWrite(valve1, !valueHigh);  // align button state with valve state (HIGHT state at valve pin == valve closed)
  if (valueHigh) {
    valveButton1 = 1;
    lastMin1 = t.min;
  } else {
    valveButton1 = 0;
    count1 = 0;
  }
}

BLYNK_WRITE(V2) {
  int valueHigh = param.asInt();
  digitalWrite(valve2, !valueHigh);
  if (valueHigh) {
    valveButton2 = 1;
    lastMin2 = t.min;
  } else {
    valveButton2 = 0;
    count2 = 0;
  }
}

BLYNK_WRITE(V3) {
  int valueHigh = param.asInt();
  digitalWrite(valve3, !valueHigh);
  if (valueHigh) {
    valveButton3 = 1;
    lastMin3 = t.min;
  } else {
    valveButton3 = 0;
    count3 = 0;
  }
}

BLYNK_WRITE(V4) {
  int valueHigh = param.asInt();
  digitalWrite(valve4, !valueHigh);
  if (valueHigh) {
    valveButton4 = 1;
    lastMin4 = t.min;
  } else {
    valveButton4 = 0;
    count4 = 0;
  }
}

BLYNK_WRITE(V5) {
  int valueHigh = param.asInt();
  digitalWrite(valve5, !valueHigh);
  if (valueHigh) {
    valveButton5 = 1;
    lastMin5 = t.min;
  } else {
    valveButton5 = 0;
    count5 = 0;
  }
}

BLYNK_WRITE(V6) {
  int valueHigh = param.asInt();
  digitalWrite(valve6, !valueHigh);
  if (valueHigh) {
    valveButton6 = 1;
    lastMin6 = t.min;
  } else {
    valveButton6 = 0;
    count6 = 0;
  }
}

void changeValveState(int valveNr, int virtualPin, bool &valveButtonOn, bool isOpen) {
  digitalWrite(valveNr, !isOpen);             // changing valve state, 0 is open
    Blynk.virtualWrite(virtualPin, isOpen);   // changing valve's button to ON (schedule run)
    valveButtonOn = isOpen;
}

void startOnCondition(int valveNr, int &lastMin, int virtualPin, bool &valveButtonOn, int startH, int startM) {
  if ((startH == t.hour) && (startM == t.min) && (!offButtonOn)) {  // start run condition
    lastMin = t.min;                                                // saving the actual minute
  changeValveState(valveNr, virtualPin, valveButtonOn, true);       // opening the valve, 0 is open
  }
}

void stopOnCondition(int valveNr, int runTime, int &count, int &lastMin, int virtualPin, bool &valveButtonOn) {
  if (!valveButtonOn) {  // is not running
    return;
  }

  if ((count < runTime) && (lastMin != t.min)) { // continue run condition and minute later
    count += 1;
    lastMin = t.min;
  }

  if ((count >= runTime) || (offButtonOn)) {                      // close valve condition
    changeValveState(valveNr, virtualPin, valveButtonOn, false);  // close valve
    count = 0;
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

  pinMode(rainSensorPin, INPUT);
  pinMode(2, OUTPUT);

  Serial.begin(9600);
  Wire.begin();
  DS3231_init(DS3231_CONTROL_INTCN);
  delay(200);

//   t.hour = 15;  // initial clock setting
//   t.min = 45;
//   t.sec = 30;
//   t.mday = 2;
//   t.mon = 6;
//   t.year = 2023;
  
  DS3231_set(t);

  BlynkEdgent.begin();
}

void loop() {
  DS3231_get(&t);
  BlynkEdgent.run();

  rainDetected = !digitalRead(rainSensorPin);  // sensor's 1 - no rain, 0 - rain

  if (rainDetected && !offButtonOn) {   // start run-disabled condition
    Blynk.virtualWrite(0, 1);           // putting on the off button ON
    offButtonOn = true;                 // the button bool variable == 1
    lastOffHour = t.hour;
  } else if (offButtonOn && (offCount < 24) && (lastOffHour != t.hour)) {  // continue run-disabled condition
    lastOffHour = t.hour;
    offCount += 1;
  } else if (offCount == 24) {  // stop  run-disabled condition
    Blynk.virtualWrite(0, 0);
    offButtonOn = false;
    offCount = 0;
  }

  // main functions
  startOnCondition(valve1, lastMin1, virtualPin1, valveButton1, startH1, startM1);
  stopOnCondition(valve1, runTime1, count1, lastMin1, virtualPin1, valveButton1);

  startOnCondition(valve2, lastMin2, virtualPin2, valveButton2, startH2, startM2);
  stopOnCondition(valve2, runTime2, count2, lastMin2, virtualPin2, valveButton2);

  startOnCondition(valve3, lastMin3, virtualPin3, valveButton3, startH3, startM3);
  stopOnCondition(valve3, runTime3, count3, lastMin3, virtualPin3, valveButton3);

  startOnCondition(valve4, lastMin4, virtualPin4, valveButton4, startH4, startM4);
  stopOnCondition(valve4, runTime4, count4, lastMin4, virtualPin4, valveButton4);

  startOnCondition(valve5, lastMin5, virtualPin5, valveButton5, startH5, startM5);
  stopOnCondition(valve5, runTime5, count5, lastMin5, virtualPin5, valveButton5);

  startOnCondition(valve6, lastMin6, virtualPin6, valveButton6, startH6, startM6);
  stopOnCondition(valve6, runTime6, count6, lastMin6, virtualPin6, valveButton6);
}
