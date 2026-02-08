
// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include "RTClib.h"

const long DATA_BAUD = 9600;
#define dataSerial Serial1

RTC_DS1307 rtc;

const uint8_t PIN_DCF77 = 3;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

int cmd = 0;
int myPins[] = {22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};

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

void setup () {

  // put your setup code here, to run once:
  for(int i = 0; i < (sizeof(myPins) / sizeof(myPins[0]));i++){
    pinMode(myPins[i], OUTPUT);    // sets the digital pin 13 as output
  }

  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial1.begin(DATA_BAUD);

  while (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    //abort();
    delay(1000);
  }
  Serial.println("RTC found");

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    rtc.adjust(DateTime(2021, 1, 1, 0, 0, 0));
  }else{
    DateTime dt = rtc.now();
    Serial.println("RTC time: ");
      displayTime();

  }



  Serial.println("Arduino - Time Receiver");
  Serial.println("============================");
  Serial.println("Waiting for time data from ESP32...");

  uint32_t test = generateInteger();
  Serial.println(test);

  bitPrint(generateInteger(),1);
  //displayTime();


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
      /*
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
      */
      //update rtc if necessary
      DateTime now = rtc.now();
      DateTime dt_serial = DateTime(currentTime.year, currentTime.month, currentTime.day, currentTime.hour, currentTime.minute, currentTime.second);

      if (abs(now.unixtime() - dt_serial.unixtime() > 5)) {
        rtc.adjust(dt_serial);
        Serial.println("\nNew RTC time");
      //} else {
      //  Serial.println("\nRTC up to date");
      }
    }
  }
}

void loop () {
  
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

  //displayTime();
  bitPrint(generateInteger(),0);
}

void displayTime() {

  Serial.println();
  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  if (now.month() < 10) {
    Serial.print("0");
  }
  Serial.print(now.month(), DEC);
  Serial.print('/');
  if (now.day() < 10) {
    Serial.print("0");
  }
  Serial.print(now.day(), DEC);
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  if (now.hour() < 10) {
    Serial.print("0");
  }
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  if (now.minute() < 10) {
    Serial.print("0");
  }
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  if (now.second() < 10) {
    Serial.print("0");
  }
  Serial.println(now.second(), DEC);
  Serial.println("");
  
}


uint32_t generateInteger() {
  DateTime now = rtc.now();
  uint8_t hh = now.hour();
  uint8_t mm = now.minute();
  uint32_t time = 0;
  //Serial.println(hh);
  //Serial.println(mm);
  switch (mm) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      time = time + 0;
      break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      time = time + 16777216;
      break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      time = time + 524288;
      break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      time = time + 3145728;
      break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
      time = time + 4194304;
      break;
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
      time = time + 29360128;
      break;
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
      hh = (hh + 1) % 24;
      time = time + 34603008;
      break;
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
      hh = (hh + 1) % 24;
      time = time + 29622272;
      break;
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
      hh = (hh + 1) % 24;
      time = time + 4456448;
      break;
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
      hh = (hh + 1) % 24;
      time = time + 2359296;
      break;
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
      hh = (hh + 1) % 24;
      time = time + 786432;
      break;
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
      hh = (hh + 1) % 24;
      time = time + 17039360;
      break;
  }
  //Serial.println(time);
  switch (hh) {
    case 0:
      time = time + 16387;
      break;
    case 1:
    case 13:
      time = time + 65603;
      break;
    case 2:
    case 14:
      time = time + 196615;
      break;
    case 3:
    case 15:
      time = time + 196627;
      break;
    case 4:
    case 16:
      time = time + 196619;
      break;
    case 5:
    case 17:
      time = time + 197635;
      break;
    case 6:
    case 18:
      time = time + 197123;
      break;
    case 7:
    case 19:
      time = time + 196739;
      break;
    case 8:
    case 20:
      time = time + 196867;
      break;
    case 9:
    case 21:
      time = time + 196643;
      break;
    case 10:
    case 22:
      time = time + 208899;
      break;
    case 11:
    case 23:
      time = time + 229379;
      break;
    case 12:
      time = time + 6147;
      break;
  }
  //Serial.println(time);
  
  return time;

}

void bitPrint(uint32_t cmd, uint32_t print)
{
  if(print)Serial.print("0b");
  for (int i = 0; i < (sizeof(myPins) / sizeof(myPins[0])); i++)
  {
    if(bitRead(cmd, i) == 1){
        if(print)Serial.print("1");
        digitalWrite(myPins[i], HIGH);
    }else{
        if(print)Serial.print("0");
        digitalWrite(myPins[i], LOW);
    }
  }
  if(print)Serial.println();
}
