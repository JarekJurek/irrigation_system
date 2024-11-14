# **Autonomous Garden Irrigation System**

This project demonstrates a modular, automated system to manage multiple irrigation sections for a garden. Built using ESP32, Blynk, Arduino UNO, and an RTC module, the system ensures efficient and timely water distribution based on user schedules and environmental inputs such as rain detection.


## **Overview**

The **Smart Garden Irrigation System** is designed to automate watering across multiple zones of a garden. It uses:
- **ESP32** for remote control and WiFi connectivity.
- **Arduino UNO** paired with a **DS3231 RTC module** for precise scheduling.
- Relays for controlling solenoid valves that regulate water flow to specific sections.
- Environmental sensors (rain sensor) to optimize water usage.

This system reduces water waste and simplifies irrigation management, making it ideal for smart gardening enthusiasts.

### System operation
* If rain is detected the system changes into not-operational state for 24h. After that time the system is again operational unless there is still water in rain senor.
* Every valve opens at specified hour and closes after time set by the slider.
* The system time from RTC is displayed in the app. If time differs significantly from the actual time, it is recommended to use the script utils/rtc_time/rtc_time.ino and other board to set it.


## **Features**

- **Multi-Zone Irrigation:** Control multiple garden sections independently.
- **Scheduled Watering:** Use RTC to ensure accurate irrigation timing.
- **Rain Detection:** Automatically suspend irrigation during rain.
- **WiFi Connectivity:** Remote monitoring and control via platforms like Blynk.
- **Scalability:** Add more zones or sensors as needed.


## **Hardware Requirements**

- **ESP32 Development Board**  
- **Arduino UNO**  
- **DS3231 RTC Module**  
- **X-Channel Relay Module** (for controlling solenoid valves)  
- **Rain Sensor**  
- **Solenoid Valves** (one per irrigation zone)  
- 12V DC Power Supply  
- Wires, resistors, and connectors  


## **Schematics**

### 1. **ESP32 Multi-Zone Irrigation Control**
This schematic showcases the connections between the ESP32, relay module, and the rain sensor to control solenoid valves across multiple garden sections.

![ESP32 Schematic](doc/pcbPrototype.png)

### 2. **Arduino UNO RTC Integration**
This schematic illustrates how the Arduino UNO interacts with the DS3231 RTC module to schedule watering tasks.

![Arduino UNO RTC Schematic](doc/UNO_RTC.png)


## **Software Setup**

### **Prerequisites**
Before starting, ensure you have the following libraries installed in your Arduino IDE:
- [Blynk Library](https://github.com/blynkkk/blynk-library) (for IoT connectivity)
- [RTClib](https://github.com/adafruit/RTClib) (for RTC module functionality; find in `lib/`)

### **Setting Up ESP32 in Arduino IDE**
To program the ESP32, you need to add the ESP32 board support to the Arduino IDE:

1. Open Arduino IDE.
2. Go to **File > Preferences**.
3. In the "Additional Boards Manager URLs" field, add the following [URL](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json)
4. Go to **Tools > Board > Boards Manager**, search for "ESP32," and install the ESP32 platform.

### **Steps to Upload Code**
1. Open the provided `.ino` files in the Arduino IDE.  
2. Select the appropriate board (**Arduino UNO** for RTC-related functions or **ESP32** for relay control).  
3. Connect the board to your computer via USB.  
4. Upload the code to the respective devices.


## **Usage**

1. **Setup Hardware:**
- Assemble the circuit as per the schematics.
- Connect solenoid valves to the relay module to control irrigation zones.

2. **Power On the System:**
- Supply 12V power to the relays and valves.
- Ensure both the ESP32 and Arduino UNO are powered and operational.

3. **Schedule Irrigation:**
- Use the Blynk app or pre-configure the Arduino code to set watering schedules.

4. **Real-Time Monitoring:**
- Check for rain sensor status to avoid unnecessary watering.


## **Contributing**

Contributions are welcome! If you have ideas for new features or improvements, feel free to submit a pull request or open an issue.


## **License**

This project is licensed under the MIT License.

