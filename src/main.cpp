#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>

// Use UART1 (GPIO 17 TX, GPIO 18 RX) for Arduino communication
// This avoids bootloader messages from UART0 interfering with the data stream
HardwareSerial SerialArduino(1);

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
const unsigned long TIME_UPDATE_INTERVAL = 15000; // Send time every 15 seconds
unsigned long lastTimeUpdate = 0;

const int LED_PIN = -1;
const bool LED_ACTIVE_LOW = false;
const int NEOPIXEL_PIN = 48;
const int NEOPIXEL_COUNT = 1;
const uint8_t NEOPIXEL_BRIGHTNESS = 32;
Adafruit_NeoPixel statusPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
const unsigned long LED_BLINK_INTERVAL = 500;
unsigned long lastLedToggle = 0;
bool ledState = false;

const char* wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_NO_SHIELD:
      return "WL_NO_SHIELD";
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "WL_UNKNOWN";
  }
}

void setLed(bool on) {
  if (NEOPIXEL_PIN >= 0) {
    uint32_t color = on ? statusPixel.Color(0, NEOPIXEL_BRIGHTNESS, 0) : 0;
    statusPixel.setPixelColor(0, color);
    statusPixel.show();
    return;
  }
  if (LED_PIN >= 0) {
    bool pinLevel = LED_ACTIVE_LOW ? !on : on;
    digitalWrite(LED_PIN, pinLevel ? HIGH : LOW);
  }
}

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
  } else {
    Serial.println("\nFailed to connect to WiFi");
    Serial.print("WiFi status: ");
    Serial.println(wifiStatusToString(WiFi.status()));
    // Scan and list available networks to help diagnose SSID visibility.
    Serial.println("Scanning for available networks...");
    int networkCount = WiFi.scanNetworks();
    if (networkCount <= 0) {
      Serial.println("No networks found");
    } else {
      for (int i = 0; i < networkCount; i++) {
        Serial.printf("%d: %s (%d dBm)%s\n",
                      i + 1,
                      WiFi.SSID(i).c_str(),
                      WiFi.RSSI(i),
                      WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? " [open]" : "");
      }
    }
  }
}

void initTime() {
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
    Serial.println();
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
  
  // Send to Arduino via UART1
  SerialArduino.println(timeString);
  
  // Also send as separate components for easier parsing on Arduino
  // Format: TIME:year,month,day,hour,minute,second
  SerialArduino.printf("TIME:%d,%d,%d,%d,%d,%d\n", 
                timeinfo.tm_year + 1900,  // Year
                timeinfo.tm_mon + 1,       // Month (0-11, so add 1)
                timeinfo.tm_mday,          // Day
                timeinfo.tm_hour,          // Hour
                timeinfo.tm_min,           // Minute
                timeinfo.tm_sec);          // Second
}

void setup() {
  // Initialize Serial (UART0) for debug output
  Serial.begin(115200);
  delay(1000);
  
  // Initialize SerialArduino (UART1) for Arduino communication
  // Using GPIO 17 (TX) and GPIO 18 (RX) to avoid bootloader messages
  SerialArduino.begin(9600, SERIAL_8N1, 18, 17);

  if (NEOPIXEL_PIN >= 0) {
    statusPixel.begin();
    statusPixel.setBrightness(NEOPIXEL_BRIGHTNESS);
    statusPixel.show();
  } else if (LED_PIN >= 0) {
    pinMode(LED_PIN, OUTPUT);
    setLed(false);
  }
  
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

  if (WiFi.status() == WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastLedToggle >= LED_BLINK_INTERVAL) {
      lastLedToggle = currentMillis;
      ledState = !ledState;
      setLed(ledState);
    }
  } else {
    ledState = false;
    setLed(false);
  }
  
  // Send time to Arduino at regular intervals
  unsigned long currentMillis = millis();
  if (currentMillis - lastTimeUpdate >= TIME_UPDATE_INTERVAL) {
    lastTimeUpdate = currentMillis;
    sendTimeToArduino();
  }
}