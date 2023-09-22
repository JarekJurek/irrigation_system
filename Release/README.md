# Irrigation
## Setup
libs required: RTC (lib/ds...), Blynk, pip install serial, 
esp32 adidional URL: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json

## System operation
* If rain is detected the system changes into not-operational state for 24h. After that time the system is again operational unless there is still water in rain senor.
* Every valve opens at specified hour and closes after specified number of minutes.

## Features TODO:
* Rain detected signal LED.
* System hour and minute for debug.
* Start-times and run-times editable