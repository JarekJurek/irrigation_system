# Irrigation
Software for managing an irrigation system using the ESP32 module and Blynk library. It initiates irrigation at preset times and runs for a duration set by a slider. The system remains inactive for 24 hours if rain is detected.

## Operation
- Start times are hardcoded and not user-adjustable.
- The rain detector operates in a normally closed state.
- Manage the RTC module using any board with the `utils/rtc_time/rtc_time.ino` script.

## Setup
Required libraries: RTC (lib/ds...), Blynk. Install using `pip install serial`.  
ESP32 additional URL: [https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json)
