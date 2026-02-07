#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

// WiFi credentials from environment variables (passed via build flags)
// If not set, falls back to placeholder values
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// NTP Server settings
const char* ntpServer = "pool.ntp.org";

// CET/CEST timezone with automatic DST handling
// CET-1CEST,M3.5.0,M10.5.0/3 means:
// - CET (UTC+1) in winter
// - CEST (UTC+2) in summer
// - DST starts: last Sunday in March at 2:00 AM
// - DST ends: last Sunday in October at 3:00 AM
const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";

// Time update interval (in milliseconds)
const unsigned long TIME_UPDATE_INTERVAL = 1000; // Send time every second
unsigned long lastTimeUpdate = 0;

void connectToWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  // Configure timezone with automatic DST handling
  setenv("TZ", timezone, 1);
  tzset();
  
  // Configure NTP
  configTime(0, 0, ntpServer);
  Serial.println("Waiting for time synchronization...");
  
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (attempts < 10) {
    Serial.println("\nTime synchronized!");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    
    // Display timezone info
    Serial.print("Timezone: ");
    Serial.print(timeinfo.tm_isdst ? "CEST (UTC+2, DST active)" : "CET (UTC+1)");
    Serial.println(
    attempts++;
  }
  
  if (attempts < 10) {
    Serial.println("\nTime synchronized!");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  } else {
    Serial.println("\nFailed to obtain time");
  }
}

void sendTimeToArduino() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  
  // Format: YYYY-MM-DD HH:MM:SS
  char timeString[64];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &timeinfo);
  
  // Send to Arduino via Serial
  Serial.println(timeString);
  
  // Also send as separate components for easier parsing on Arduino
  // Format: TIME:year,month,day,hour,minute,second
  Serial.printf("TIME:%d,%d,%d,%d,%d,%d\n", 
                timeinfo.tm_year + 1900,  // Year
                timeinfo.tm_mon + 1,       // Month (0-11, so add 1)
                timeinfo.tm_mday,          // Day
                timeinfo.tm_hour,          // Hour
                timeinfo.tm_min,           // Minute
                timeinfo.tm_sec);          // Second
}

void setup() {
  // Initialize Serial for communication with Arduino Mega
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("ESP32 Internet Time to Arduino");
  Serial.println("================================");
  
  // Connect to WiFi
  connectToWiFi();
  
  // Initialize and synchronize time with NTP server
  if (WiFi.status() == WL_CONNECTED) {
    initTime();
  }
}

void loop() {
  // Check if WiFi is still connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection lost. Reconnecting...");
    connectToWiFi();
    if (WiFi.status() == WL_CONNECTED) {
      initTime();
    }
  }
  
  // Send time to Arduino at regular intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    lastTimeUpdate = currentMillis;
    sendTimeToArduino();
  }
}