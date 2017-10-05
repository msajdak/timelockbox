
#include <Button.h>
#include <stdio.h>
#include <DS3231.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // dolaczenie pobranej biblioteki I2C dla LCD
#include <EEPROM.h>

#define MAX_OUT_CHARS 16

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Ustawienie adresu ukladu na 0x27

DS3231 Clock;

byte padlockOpen[8] = {
  0b01110,
  0b10001,
  0b10001,
  0b10000,
  0b10000,
  0b11111,
  0b11111,
  0b11111
};
byte padlockLock[8] = {
  0b00000,
  0b01110,
  0b10001,
  0b10001,
  0b10001,
  0b11111,
  0b11111,
  0b11111
};
/*
  byte time[8] = {
  0b00000,
  0b01110,
  0b10101,
  0b10111,
  0b10001,
  0b01110,
  0b00000,
  0b00000
  };
*/

byte arrowup[8] = {
  0b00000,
  0b00100,
  0b01110,
  0b10101,
  0b00100,
  0b00100,
  0b00100,
  0b00000
};
byte bunny1[8] = {
  0b00001,
  0b00011,
  0b11101,
  0b01111,
  0b01111,
  0b00011,
  0b00000,
  0b00001
};

byte bunny2[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b10000,
  0b11100,
  0b11110,
  0b11111,
  0b11111
};

byte bunny3[8] = {
  0b00001,
  0b00011,
  0b00111,
  0b00111,
  0b00111,
  0b01111,
  0b11111,
  0b11111
};
byte bunny4[8] = {
  0b11110,
  0b11100,
  0b11100,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11100
};


const char DDMMYYYYHHMM[] PROGMEM = {  "%02d.%02d.%02d %02d%c%02d %c"};

const char SETUP_TIME[] PROGMEM = {  " Setup Time"};
const char SETUP_OPEN[] PROGMEM = {  " Setup Open Time"};

const char YEAR[] PROGMEM = "year";
const char MONTH[] PROGMEM = "month";
const char DAY[] PROGMEM = "day";
const char HOUR[] PROGMEM = "hour";
const char MINUTE[] PROGMEM = "minute";

const char SAVED[] PROGMEM = "Saved";
const char CANCELED[] PROGMEM = "Canceled";

const char RESSETING[] PROGMEM = "Reseting";
const char OPENING[] PROGMEM = "OPENING";
const char LOCK[] PROGMEM = "LOCK";

const char BUNNY1[] PROGMEM = "\x03\x04";
const char BUNNY2[] PROGMEM = "\x05\x06";

uint8_t openTime[6] = {};
uint8_t currentTime[6] = {};

Button b1 = Button(2, BUTTON_PULLUP_INTERNAL);
Button b2 = Button(3, BUTTON_PULLUP_INTERNAL);
Button b3 = Button(4, BUTTON_PULLUP_INTERNAL);
Button b4 = Button(5, BUTTON_PULLUP_INTERNAL);

Button b5 = Button(6, BUTTON_PULLUP_INTERNAL);
Button b6 = Button(7, BUTTON_PULLUP_INTERNAL);

Button bReset = Button(10, BUTTON_PULLUP_INTERNAL);

int magnetPin=17;

bool isOpen();
void readOpenTime();
bool setTime(uint8_t time[]);


void setup() {
  
  // Start the I2C interface
  Wire.begin();

  pinMode(12, OUTPUT);        
  pinMode(11, OUTPUT);        
  digitalWrite(12, HIGH);
 digitalWrite(11, LOW);

  lcd.begin(16, 2); // Inicjalizacja LCD 2x16

  lcd.createChar(1, padlockLock);
  lcd.createChar(2, arrowup);
  lcd.createChar(3, bunny1);
  lcd.createChar(4, bunny2);
  lcd.createChar(5, bunny3);
  lcd.createChar(6, bunny4);
  lcd.createChar(7, padlockOpen);

 pinMode(magnetPin, OUTPUT);
 digitalWrite(magnetPin, LOW);  
  readCurrentTime();
  readOpenTime();
}

void print(int c, int r,
           const char * toPrint) {
  char buffer[MAX_OUT_CHARS + 1];
  lcd.setCursor(c, r);
  sprintf_P(buffer, toPrint);
  lcd.print(buffer);
}


void printBunny(int col) {
 
  char buffer[MAX_OUT_CHARS + 1];

   lcd.setCursor(col, 0);
  sprintf_P(buffer, BUNNY1);
  lcd.print(buffer);

     lcd.setCursor(col, 1);
  sprintf_P(buffer, BUNNY2);
  lcd.print(buffer);
}
void printInfo( const char * toPrint,boolean pause) {
  lcd.clear();
  char buffer[MAX_OUT_CHARS + 1];
  lcd.setCursor(0, 0);
  sprintf_P(buffer, toPrint);
  lcd.print(buffer);
  if (pause){
    delay(500);
  }
}

void printTime(int c, char icon, uint8_t time[]) {
  char buffer[MAX_OUT_CHARS + 1];
  lcd.setCursor(0, c);
  sprintf_P(buffer, DDMMYYYYHHMM, time[2], time[1], time[0], time[3], time[5] % 2 ? ':' : ' ', time[4], icon);
  lcd.print(buffer);
}

void printCurrentTime() {
  printTime(0, ' ', currentTime);
}

void printOpenTime() {
  printTime(1, isOpen() ? '\x07' : '\x01', openTime);

}

unsigned int getITime(uint8_t time[]) {
  int ret=time[0] * 12;
  ret=(ret+ time[1])*31;
  ret=(ret+ time[2])*24;
  ret=(ret+ time[3])*60;
   ret=ret+ time[4];
  return ret;
}

bool sessionOpen=false;
bool isOpen() {
  if (sessionOpen){
    return true;
  }
  if (currentTime[0] <= 1) {
    return true;
  }
  sessionOpen=getITime(currentTime) >= getITime(openTime);
  return sessionOpen;

}

void readCurrentTime() {
  bool Century = false;
  bool h12;
  bool PM;

  currentTime[5] = Clock.getSecond();
  currentTime[4] = Clock.getMinute();
  currentTime[3] = Clock.getHour(h12, PM);
  currentTime[2] = Clock.getDate();
  currentTime[1] = Clock.getMonth(Century);
  currentTime[0] = Clock.getYear();

}


#define increase(X, minI, maxI) X = (X >= maxI) ? minI : X+1
#define decrease(X, minI, maxI) X = (X <= minI) ? maxI : X-1

void showMenu() {
  uint8_t currentPos = 1;
  bool first = true;
  lcd.clear();
  while (1) {

    if (b1.isPressed() || b3.isPressed()) {
      currentPos = increase(currentPos, 0, 1);
    }

    if (b2.isPressed() || b4.isPressed()) {
      currentPos = decrease(currentPos, 0, 1);
    }


    if (b5.isPressed() ) {
      if (currentPos == 0) {
        if (setTime(currentTime)) {
          setCurrentTime();
          printInfo(SAVED,1);
        } else {
          printInfo(CANCELED,1);
        }
      } else {
        if (setTime(openTime)) {
          saveOpenTime();
          printInfo(SAVED,1);
        } else {
          printInfo(CANCELED,1);
         
        }

      }
      
      return;
    }
    if (b6.isPressed() && !first ) {
      return;
    }



    if (b1.isPressed() || b2.isPressed() || b3.isPressed() || b4.isPressed() || b5.isPressed() || b6.isPressed() || first) {
 
      lcd.setCursor(0, 0);
      print(0, 0, SETUP_TIME);
      lcd.setCursor(1, 0);
      print(0, 1, SETUP_OPEN);

      lcd.setCursor(0, currentPos);

      lcd.print("\x7E");


      if (first) {
        delay(1000);
        first = false;
      } else {
        delay(400);
      }

    }

  }
}

bool setTime(uint8_t time[]) {

  lcd.clear();

  const uint8_t maxV[] = {    99,    12,    31,    24,    59  };
  const uint8_t minV[] = {    0,    1,    1,    0,    0  };
  const uint8_t arrowPos[] = {    6,    3,    0,    9,    12  };

  uint8_t currentPos = 2;
  delay(1000);
  bool first = true;
  while (1) {

    if (b1.isPressed()) {
      time[currentPos] = increase(time[currentPos], minV[currentPos], maxV[currentPos]);
    }

    if (b2.isPressed()) {
      time[currentPos] = decrease(time[currentPos], minV[currentPos], maxV[currentPos]);
    }


    if (b3.isPressed()) {
      currentPos = increase(currentPos, 0, 4);

    }

    if (b4.isPressed()) {
      currentPos = decrease(currentPos, 0, 4);
    }



    if (b5.isPressed()) {
      return 1;
    }

    if (b6.isPressed()) {
      return 0;
    }

    if (b1.isPressed() || b2.isPressed() || b3.isPressed() || b4.isPressed() || b5.isPressed() || b6.isPressed() || first) {



      printTime(0, ' ', time);

      lcd.setCursor(0, 1);
      lcd.print("                ");

      uint8_t r = 8;
      if (arrowPos[currentPos] > r) {
        r = 0;
      }

      switch (currentPos) {
        case 0:
          print(r, 1, YEAR);
          break;
        case 1:
          print(r, 1, MONTH);
          break;
        case 2:
          print(r, 1, DAY);
          break;
        case 3:
          print(r, 1, HOUR);
          break;
        case 4:
          print(r, 1, MINUTE);
          break;
      }

      lcd.setCursor(arrowPos[currentPos], 1);
      lcd.print("\x02\x02");

      first = false;
      delay(200);
    }

  }
}

void setCurrentTime() {
  Clock.setSecond(0); //Set the second
  Clock.setMinute(currentTime[4]); //Set the minute
  Clock.setHour(currentTime[3]); //Set the hour
  Clock.setDate(currentTime[2]); //Set the date of the month
  Clock.setMonth(currentTime[1]); //Set the month of the year
  Clock.setYear(currentTime[0]); //Set the year (Last two digits of the year)
}


void readOpenTime() {

  for (int k = 0; k < 5; k++)
  {
    EEPROM.get(k * 4,  openTime[k] );
    if (openTime[k] == 255) {
      openTime[k] = currentTime[k];
    }
  }
  openTime[5] = 1;

}

void saveOpenTime() {
  openTime[5] = 0;
  for (int k = 0; k < 5; k++)
  {
    EEPROM.put(k * 4, openTime[k] );
  }

}


void resetOpenTime() {
  
  for (int k = 0; k < 4; k++)
  {
   openTime[k]=currentTime[k];
  }
   openTime[4]=currentTime[4]+1;
    printInfo(RESSETING,1);
}

uint16_t loopI=0;
void loop() {

  if (loopI%20==0){
    readCurrentTime();
    printCurrentTime();
    printOpenTime();
  }


if (bReset.isPressed()){
  resetOpenTime();
 
}

if (isOpen()){
  if (b6.isPressed()) {
    
    showMenu();
    lcd.clear();
     printBunny(2);
     printBunny(6);
    printBunny(10);
    printBunny(14);
    delay(1000);
     loopI=0;
  }else if (b1.isPressed() || b2.isPressed() || b3.isPressed() || b4.isPressed() || b5.isPressed()){
    digitalWrite(magnetPin, HIGH);  
    printInfo(OPENING,0);
    printBunny(14);
    delay(1000);
    digitalWrite(magnetPin, LOW);  
    loopI=0;
  }
}else if (b1.isPressed() || b2.isPressed() || b3.isPressed() || b4.isPressed() || b5.isPressed() || b6.isPressed()){
   printInfo(LOCK,0);
   printBunny(14);
   loopI=0;

   delay(1000);
}


  if (loopI==3000){//30s
    lcd.setBacklight(BACKLIGHT_OFF);
     lcd.clear();
       printBunny(0);
            printBunny(3);
            printBunny(6);
            printBunny(9);
            printBunny(12);
            
    while(1){
         if (b1.isPressed() || b2.isPressed() || b3.isPressed() || b4.isPressed() || b5.isPressed() || b6.isPressed()|| bReset.isPressed()){
            lcd.setBacklight(BACKLIGHT_ON);
         
            loopI=0;
            break;
         }
         delay(1000);
    }
  }

  loopI++;
  
  delay(10);

}
