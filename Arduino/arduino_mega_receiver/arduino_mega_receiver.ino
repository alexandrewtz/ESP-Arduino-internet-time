/*
 * Arduino Mega / Uno - Time Receiver from ESP32
 * 
 * This sketch receives time data from ESP32 via Serial.
 *
 * Mega 2560 wiring:
 *   ESP32 GPIO 17 (TX) -> Mega RX1 (pin 19)
 *   ESP32 GND -> Mega GND
 *
 * Uno wiring (SoftwareSerial):
 *   ESP32 GPIO 17 (TX) -> Uno D2 (RX)
 *   ESP32 GND -> Uno GND
 *   Do NOT connect Uno TX (5V) to ESP32 RX unless level shifted.
 * 
 * The ESP32 sends time in two formats:
 * 1. Human readable: "YYYY-MM-DD HH:MM:SS"
 * 2. Parseable: "TIME:year,month,day,hour,minute,second"
 */

#if defined(ARDUINO_AVR_UNO)
#include <SoftwareSerial.h>
const int RX_PIN = 2;
const int TX_PIN = 3;
const long DATA_BAUD = 9600;
SoftwareSerial dataSerial(RX_PIN, TX_PIN);
#elif defined(ARDUINO_AVR_MEGA2560)
const long DATA_BAUD = 9600;
#define dataSerial Serial1
#else
#include <SoftwareSerial.h>
const int RX_PIN = 2;
const int TX_PIN = 3;
const long DATA_BAUD = 9600;
SoftwareSerial dataSerial(RX_PIN, TX_PIN);
#endif

// Structure to store time data
struct TimeData {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  bool valid;
};

TimeData currentTime = {0, 0, 0, 0, 0, 0, false};

void setup() {
  // Serial for debugging (USB connection)
  Serial.begin(115200);
  
  // Data serial for receiving from ESP32
  #if defined(ARDUINO_AVR_UNO) || !defined(ARDUINO_AVR_MEGA2560)
  dataSerial.begin(DATA_BAUD);
  #else
  Serial1.begin(DATA_BAUD);
  #endif
  
  Serial.println("Arduino - Time Receiver");
  Serial.println("============================");
  Serial.println("Waiting for time data from ESP32...");
}

void parseTimeData(String data) {
  // Parse format: "TIME:year,month,day,hour,minute,second"
  if (data.startsWith("TIME:")) {
    data.remove(0, 5); // Remove "TIME:" prefix
    
    int values[6];
    int index = 0;
    int lastComma = -1;
    
    for (int i = 0; i < data.length() && index < 6; i++) {
      if (data.charAt(i) == ',' || i == data.length() - 1) {
        String valueStr;
        if (i == data.length() - 1) {
          valueStr = data.substring(lastComma + 1);
        } else {
          valueStr = data.substring(lastComma + 1, i);
        }
        values[index++] = valueStr.toInt();
        lastComma = i;
      }
    }
    
    if (index == 6) {
      currentTime.year = values[0];
      currentTime.month = values[1];
      currentTime.day = values[2];
      currentTime.hour = values[3];
      currentTime.minute = values[4];
      currentTime.second = values[5];
      currentTime.valid = true;
      
      // Display the received time
      Serial.print("Received time: ");
      Serial.print(currentTime.year);
      Serial.print("-");
      if (currentTime.month < 10) Serial.print("0");
      Serial.print(currentTime.month);
      Serial.print("-");
      if (currentTime.day < 10) Serial.print("0");
      Serial.print(currentTime.day);
      Serial.print(" ");
      if (currentTime.hour < 10) Serial.print("0");
      Serial.print(currentTime.hour);
      Serial.print(":");
      if (currentTime.minute < 10) Serial.print("0");
      Serial.print(currentTime.minute);
      Serial.print(":");
      if (currentTime.second < 10) Serial.print("0");
      Serial.println(currentTime.second);
    }
  }
}

void loop() {
  // Check if data is available from ESP32
  if (dataSerial.available()) {
    String incomingData = dataSerial.readStringUntil('\n');
    incomingData.trim();
    
    if (incomingData.length() > 0) {
      // Check if it's the parseable format
      if (incomingData.startsWith("TIME:")) {
        parseTimeData(incomingData);
      } else {
        // It's the human-readable format, just display it
        Serial.println("Time: " + incomingData);
      }
    }
  }
  
  // You can use currentTime.year, currentTime.month, etc. in your code
  // The valid flag indicates if we've received at least one time update
}
