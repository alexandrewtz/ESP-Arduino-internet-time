int myPins[] = {22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47};
uint8_t hh;
uint8_t mm;

void setup () {

  // put your setup code here, to run once:
  for(int i = 0; i < (sizeof(myPins) / sizeof(myPins[0]));i++){
    pinMode(myPins[i], OUTPUT);    // sets the digital pin 13 as output
  }

  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  mm=0;
  hh=0;

}


void loop () {
  
  //mm += 1;
  //if(mm >= 60)hh+=1;
  //mm = mm%60;
  hh+=1;
  hh = hh%24;
  
  bitPrint(generateInteger(hh,mm),1);

  delay(1000); // wait 0.1 second
}




uint32_t generateInteger(uint8_t hh, uint8_t mm) {
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
