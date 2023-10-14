#include <Wire.h>
#include "ds3231.h"

struct ts t;  // Time structure for the RTC

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // // Uncomment the lines below to set the time initially
  // t.hour = 14;  // Set the initial time here
  // t.min = 15;
  // t.sec = 0;
  // t.mday = 14;  // Day of the month
  // t.mon = 10;    // Month
  // t.year = 2023; // Year
  // t.wday = 6;   // Day of the week: 0 = Sunday, 1 = Monday, etc.
  
  // // Set the time
  // DS3231_set(t);

  // Initialize the RTC
  DS3231_init(DS3231_CONTROL_INTCN);
}

void loop() {
  // Get the current time
  DS3231_get(&t);

  // Print the current time to the Serial Monitor
  Serial.print("Time: ");
  Serial.print(t.hour);
  Serial.print(":");
  Serial.print(t.min);
  Serial.print(":");
  Serial.println(t.sec);

  delay(1000); // Wait for a second
}
