#include "Blynk.h"
#include <Wire.h>
#include "ds3231.h"

struct ts t;  // Time structure for the RTC

BlynkTimer timer;

void updateBlynkTime() {
  DS3231_get(&t);
  publishTime(t.hour, t.min, t.sec);
}

void publishTime(int hour, int minute, int sec) {
  Serial.print("Time: ");
  Serial.print(hour);
  Serial.print(":");
  Serial.print(minute);
  Serial.print(":");
  Serial.println(sec);
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Uncomment the lines below to set the time initially
  // t.hour = 15;  // Set the initial time here
  // t.min = 58;
  // t.sec = 0;
  // t.mday = 14;  // Day of the month
  // t.mon = 10;    // Month
  // t.year = 2023; // Year
  // t.wday = 6;   // Day of the week: 0 = Sunday, 1 = Monday, etc.
  // DS3231_set(t); // set time

  DS3231_init(DS3231_CONTROL_INTCN);
  timer.setInterval(1000L, updateBlynkTime);
}

void loop() {
  timer.run();
}
