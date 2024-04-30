#include "Diagnosis.h"

#define UP 1
#define DOWN 2
#define ENTER 3
#define RESET 4
#define TEST 5

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

const int _doorLock    = 5;
const int _redLEDPin   = 6;
const int _greenLEDPin = 7;
const int _blueLEDPin  = 8;
const int _dTrig       = 14;
const int _failNC      = 15;
const int _rfidPins[]  = {18, 19, 23, 25, 27, 29};
const int _button[]    = {44, 42, 45, 46, 47};
const int _ePin        = 37;
const int _mPin        = 39;
const int _hPin        = 41;
const int _bypassPin   = 49;
const int _relay1      = 9;
const int _relay2      = 48;

int _prev[] = { 0,  0,  0,  0,  0};
bool printed = false;
int position = 1;
unsigned long autoscrollTimer;

void(* resetFunc) (void) = 0;

void diagnosisSetup(){
  pinMode(_doorLock,    OUTPUT);
  pinMode(_redLEDPin,   OUTPUT);
  pinMode(_greenLEDPin, OUTPUT);
  pinMode(_blueLEDPin,  OUTPUT);
  pinMode(_dTrig,        INPUT);
  pinMode(_failNC,       INPUT);
  pinMode(_relay1,      OUTPUT);
  pinMode(_relay2,      OUTPUT);
  pinMode(_ePin,        OUTPUT);
  pinMode(_mPin,        OUTPUT);
  pinMode(_hPin,        OUTPUT);
  pinMode(_bypassPin,   OUTPUT);

  for(int i=0; i<5; i++){
    pinMode(_button[i], INPUT);
  }

  for (int i=0; i<6; i++){
    pinMode(_rfidPins[i], INPUT);
  }

  lcd.begin();
  lcd.clear();
  lcd.print("Normal Mode |       ");
  lcd.setCursor(0,1);
  lcd.print("Room: ");
  lcd.print(roomName);
  lcd.noBacklight();

  delay(1000);
}

//---------------Testing Mode-------------------
void diagnosisStart(){
  Serial.println("Diagnosis Mode");
  digitalWrite(_doorLock, HIGH);
  lcd.clear();
  lcd.backlight();
  printed = false;
  position = 1;
  
  while(digitalRead(diagnosisTestBtn) == 1){
    room.lightOff();
    digitalWrite(_doorLock, HIGH);
    int press = _buttonListener();
    if (press == UP){    //up
      position--;
      if (position < 1){
        position = MENU_ENTRIES;
      }
      printed = false;
    }
  
    if (press == DOWN){    //down
      position++;
      if (position > MENU_ENTRIES){
        position = 1;
      }
      printed = false;
    }
  
    if (press == ENTER){    //enter
      delay(200);
      if (position == 1)
        checkConnection();
      else if (position == 2)
        checkRGBLedTape();
      else if (position == 3)
        check_doorLock();
      else if (position == 4)
        checkDoorState();
      else if (position == 5)
        checkBoardRelays();
      printed = false;
    }
  
    if (press == RESET) { resetFunc(); }
  
    if (!printed){
      lcd.clear();
      lcd.print("Testing | Main Menu ");
      switch(position){
        case 1:
          lcd.setCursor(0,1);
          lcd.print(">Check connection to");
          lcd.setCursor(0,2);
          lcd.print(" database           ");
          lcd.setCursor(0,3);
          lcd.print(" Check RGB led tape ");
          break;
  
        case 2:
          lcd.setCursor(0,1);
          lcd.print(" Check connection to");
          lcd.setCursor(0,2);
          lcd.print(" database           ");
          lcd.setCursor(0,3);
          lcd.print(">Check RGB led tape ");
          break;
  
        case 3:
          lcd.setCursor(0,1);
          lcd.print(">Check door lock    ");
          lcd.setCursor(0,2);
          lcd.print(" Check door state   ");
          lcd.setCursor(0,3);
          lcd.print(" Check relays       ");
          break;
  
        case 4:
          lcd.setCursor(0,1);
          lcd.print(" Check door lock    ");
          lcd.setCursor(0,2);
          lcd.print(">Check door state   ");
          lcd.setCursor(0,3);
          lcd.print(" Check relays       ");
          break;

        case 5:
          lcd.setCursor(0,1);
          lcd.print(" Check door lock    ");
          lcd.setCursor(0,2);
          lcd.print(" Check door state   ");
          lcd.setCursor(0,3);
          lcd.print(">Check relays       ");
          break;
      }
      printed = true;
    }
  
    delay(50);
  }

  digitalWrite(_doorLock, LOW);
  lcd.clear();
  lcd.print("Normal Mode |       ");
  lcd.setCursor(0,1);
  lcd.print("Room: ");
  lcd.print(roomName);
  lcd.noBacklight();
}

int _buttonListener(){
  bool pressed[] = {false, false, false, false, false};
  
  room.pingAgent();

  for (int i=0; i<5; i++){
    int val = digitalRead(_button[i]);
    if (val == 1){
      if (_prev[i] == 0){
        pressed[i] = true;
      }
      else{
        pressed[i] = false; 
      }
      _prev[i] = 1;
    }
    else{
      _prev[i] = 0;
      pressed[i] = false;
    }
  }

  for (int i=0; i<5; i++){
    if (pressed[i])
      return i+1;
  }

  return 0;
}

void checkConnection(){
  int rs = room.getRoomStatusFromSerial();
  lcd.clear();
  lcd.print("Testing | Connection");
  lcd.setCursor(0,1);
  lcd.print("Room: ");
  lcd.print(roomName);
  lcd.setCursor(0,2);
  lcd.print("status: ");
  switch(rs){
    case 0:
      lcd.print("active");
      break;
      
    case 1:
      lcd.print("activated");
      break;
      
    case 2:
      lcd.print("win");
      break;
      
    case 3:
      lcd.print("lose");
      break;
      
    case 4:
      lcd.print("timeout");
      break;
      
    case 5:
      lcd.print("emergency");
      break;
      
    case 6:
      lcd.print("inactive");
      break;
      
    case 7:
      lcd.print("door");
  }
  lcd.setCursor(0,3);
  lcd.print(">back");
  while (_buttonListener() != ENTER){ 
    agentLoop.update(); 
    delay(100);
  }
}

//-----------Check RGB Led Tape-----------------
void checkRGBLedTape(){
  int state = 1;    //state: 1=red, 2=green, 3=blue, 4=cube, 5=exit
  lcd.clear();
  lcd.print("Testing | RGB Led   ");
  lcd.setCursor(0,1);
  lcd.print(">Red     Cube       ");
  lcd.setCursor(0,2);
  lcd.print(" Green   Off        ");
  lcd.setCursor(0,3);
  lcd.print(" Blue    back       ");

  bool exit = false;
  while (!exit){
    delay(50);
    int ret = _buttonListener();

    if (ret == UP){
      state--;
      if (state < 1)
        state = 6;
    }

    if (ret == DOWN){
      state++;
      if (state > 6)
        state = 1;
    }

    if (ret == ENTER){
      if (state == 1)
        lightRed();
      else if (state == 2)
        lightGreen();
      else if (state == 3)
        lightBlue();
      else if (state == 4)
        lightCube();
      else if (state == 5)
        lightOff();
      else
        exit = true;
      
    }

    if (ret != 0){
      switch (state){
        case 1:
          lcd.setCursor(0,1);
          lcd.print(">");
          lcd.setCursor(0,2);
          lcd.print(" ");
          lcd.setCursor(0,3);
          lcd.print(" ");
          lcd.setCursor(8,1);
          lcd.print(" ");
          lcd.setCursor(8,2);
          lcd.print(" ");
          lcd.setCursor(8,3);
          lcd.print(" ");
          break;
        
        case 2:
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.setCursor(0,2);
          lcd.print(">");
          lcd.setCursor(0,3);
          lcd.print(" ");
          lcd.setCursor(8,1);
          lcd.print(" ");
          lcd.setCursor(8,2);
          lcd.print(" ");
          lcd.setCursor(8,3);
          lcd.print(" ");
          break;

        case 3:
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.setCursor(0,2);
          lcd.print(" ");
          lcd.setCursor(0,3);
          lcd.print(">");
          lcd.setCursor(8,1);
          lcd.print(" ");
          lcd.setCursor(8,2);
          lcd.print(" ");
          lcd.setCursor(8,3);
          lcd.print(" ");
          break;

        case 4:
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.setCursor(0,2);
          lcd.print(" ");
          lcd.setCursor(0,3);
          lcd.print(" ");
          lcd.setCursor(8,1);
          lcd.print(">");
          lcd.setCursor(8,2);
          lcd.print(" ");
          lcd.setCursor(8,3);
          lcd.print(" ");
          break;

        case 5:
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.setCursor(0,2);
          lcd.print(" ");
          lcd.setCursor(0,2);
          lcd.print(" ");
          lcd.setCursor(8,1);
          lcd.print(" ");
          lcd.setCursor(8,2);
          lcd.print(">");
          lcd.setCursor(8,3);
          lcd.print(" ");
          break;

        case 6:
          lcd.setCursor(0,1);
          lcd.print(" ");
          lcd.setCursor(0,2);
          lcd.print(" ");
          lcd.setCursor(0,3);
          lcd.print(" ");
          lcd.setCursor(8,1);
          lcd.print(" ");
          lcd.setCursor(8,2);
          lcd.print(" ");
          lcd.setCursor(8,3);
          lcd.print(">");
      }
    }
  }
  lightOff();
}

//------------------RGB States------------------
void lightRed() {
  digitalWrite(_redLEDPin,   HIGH);
  digitalWrite(_greenLEDPin, LOW);
  digitalWrite(_blueLEDPin,  LOW);
}

void lightGreen() {
  digitalWrite(_redLEDPin,   LOW);
  digitalWrite(_greenLEDPin, HIGH);
  digitalWrite(_blueLEDPin,  LOW);
}

void lightBlue() {
  digitalWrite(_redLEDPin,   LOW);
  digitalWrite(_greenLEDPin, LOW);
  digitalWrite(_blueLEDPin,  HIGH);
}

void lightCube() {
  digitalWrite(_redLEDPin,   LOW);
  digitalWrite(_greenLEDPin, HIGH);
  digitalWrite(_blueLEDPin,  HIGH);
}

void lightOff() {
  digitalWrite(_redLEDPin,   LOW);
  digitalWrite(_greenLEDPin, LOW);
  digitalWrite(_blueLEDPin,  LOW);
}

void check_doorLock(){
  int state = 1;

  lcd.clear();
  lcd.print("Testing | Door Lock");
  lcd.setCursor(0,1);
  lcd.print("Set: >Unlock");
  lcd.setCursor(0,2);
  lcd.print("      Lock");
  lcd.setCursor(0,3);
  lcd.print(" back");

  bool exit = false;

  while (!exit){
    delay(50);
    int ret = _buttonListener();

    if (ret == UP){
      state--;
      if (state < 1)
        state = 3;
    }

    if (ret == DOWN){
      state++;
      if (state > 3)
        state = 1;
    }
      
    if (ret != 0){
      switch (state){
        case 1:
          lcd.setCursor(5,1);
          lcd.print(">");
          lcd.setCursor(5,2);
          lcd.print(" ");
          lcd.setCursor(0,3);
          lcd.print(" ");
          break;

        case 2:
          lcd.setCursor(5,1);
          lcd.print(" ");
          lcd.setCursor(5,2);
          lcd.print(">");
          lcd.setCursor(0,3);
          lcd.print(" ");
          break;

        case 3:
          lcd.setCursor(5,1);
          lcd.print(" ");
          lcd.setCursor(5,2);
          lcd.print(" ");
          lcd.setCursor(0,3);
          lcd.print(">");
      }
    }

    if (ret == ENTER){
      switch (state)
      {
      case 1:
        digitalWrite(_doorLock, HIGH);
        break;
      
      case 2:
        digitalWrite(_doorLock, LOW);
        break;

      case 3:
        digitalWrite(_doorLock, LOW);
        exit = true;
      }
    }
  }
}

void checkDoorState(){
  bool exit = false;

  lcd.clear();
  lcd.print("Testing | Door State");
  lcd.setCursor(0,1);
  lcd.print("door is: ");
  lcd.setCursor(0,2);
  lcd.print(">back");
  
  while (!exit){
    delay(100);

    uint8_t val = digitalRead(_dTrig);
    lcd.setCursor(9,1);
    if (val == 0)
      lcd.print("open  ");
    else
      lcd.print("closed");

    if (_buttonListener() == ENTER)
      exit = true;
  }
}

void checkFailRadars(){
  bool exit = false;
  lcd.clear();
  lcd.print("Testing | Radars");
  lcd.setCursor(0,1);
  lcd.print("State: ");
  lcd.setCursor(0,2);
  lcd.print(">back");

  while (!exit){
    delay(50);

    int ret = _buttonListener();
    if (ret == ENTER)
      exit = true;

    int val = digitalRead(_failNC);
    lcd.setCursor(7,1);
    lcd.print(val);

    if (val == 0)
      lightRed();
    else
      lightGreen();
  }

  lightOff();
}

void checkRfidFinalStation(){
  bool exit = false;
  lcd.clear();
  lcd.print("Testing | RFID Final");
  lcd.setCursor(0,2);
  lcd.print(">back");

  while(!exit){
    delay(50);

    if (_buttonListener() == ENTER)
      exit = true;

    int counter = 0;

    for (int i=0; i<6; i++){
      int val = digitalRead(_rfidPins[i]);
      lcd.setCursor(i*2,1);
      lcd.print(val);
      if (val == 1)
        counter++;
    }

    if (counter > 0)
      lightGreen();
    else
      lightOff();
  }
}

void checkBoardRelays(){
  bool exit = false;
  int state = 1;

  lcd.clear();
  lcd.print("Testing | Relays");
  lcd.setCursor(0,1);
  lcd.print("1:>on  off");
  lcd.setCursor(0,2);
  lcd.print("2: on  off");
  lcd.setCursor(0,3);
  lcd.print(" back");

  while(!exit){
    delay(50);

    int ret = _buttonListener();

    if (ret == UP){
      state--;
      if (state < 1)
        state = 5;
    }
    else if (ret == DOWN){
      state++;
      if (state > 5)
        state = 1;
    }

    if (state == 1){
      lcd.setCursor(2,1);
      lcd.print(">");
      lcd.setCursor(6,1);
      lcd.print(" ");
      lcd.setCursor(2,2);
      lcd.print(" ");
      lcd.setCursor(6,2);
      lcd.print(" ");
      lcd.setCursor(0,3);
      lcd.print(" ");
    } 
    else if (state == 2){
      lcd.setCursor(2,1);
      lcd.print(" ");
      lcd.setCursor(6,1);
      lcd.print(">");
      lcd.setCursor(2,2);
      lcd.print(" ");
      lcd.setCursor(6,2);
      lcd.print(" ");
      lcd.setCursor(0,3);
      lcd.print(" ");
    }
    else if (state == 3){
      lcd.setCursor(2,1);
      lcd.print(" ");
      lcd.setCursor(6,1);
      lcd.print(" ");
      lcd.setCursor(2,2);
      lcd.print(">");
      lcd.setCursor(6,2);
      lcd.print(" ");
      lcd.setCursor(0,3);
      lcd.print(" ");
    }  
    else if (state == 4){
      lcd.setCursor(2,1);
      lcd.print(" ");
      lcd.setCursor(6,1);
      lcd.print(" ");
      lcd.setCursor(2,2);
      lcd.print(" ");
      lcd.setCursor(6,2);
      lcd.print(">");
      lcd.setCursor(0,3);
      lcd.print(" ");
    }
    else{
      lcd.setCursor(2,1);
      lcd.print(" ");
      lcd.setCursor(6,1);
      lcd.print(" ");
      lcd.setCursor(2,2);
      lcd.print(" ");
      lcd.setCursor(6,2);
      lcd.print(" ");
      lcd.setCursor(0,3);
      lcd.print(">");
    }

    if (ret == ENTER){
      if (state == 1)
        digitalWrite(_relay1, HIGH);
      else if (state == 2)
        digitalWrite(_relay1, LOW);
      else if (state == 3)
        digitalWrite(_relay2, HIGH);
      else if (state == 4)
        digitalWrite(_relay2, LOW);
      else
        exit = true;
    }    
  }
  digitalWrite(_relay1, LOW);
  digitalWrite(_relay2, LOW);
}

void checkMotors(){
  bool exit = false;
  int state = 1;

  lcd.clear();
  lcd.print("Testing | Motors");
  lcd.setCursor(0,1);
  lcd.print("Difficulty: >easy  ");
  lcd.setCursor(0,2);
  lcd.print("             medium");
  lcd.setCursor(0,3);
  lcd.print(" back        hard  ");
  while(!exit){
    delay(50);
    int ret = _buttonListener();

    switch (state){
      case 1:
        lcd.setCursor(12, 1);
        lcd.print(">");
        lcd.setCursor(12, 2);
        lcd.print(" ");
        lcd.setCursor(12, 3);
        lcd.print(" ");
        lcd.setCursor(0, 3);
        lcd.print(" ");
        break;

      case 2:
        lcd.setCursor(12, 1);
        lcd.print(" ");
        lcd.setCursor(12, 2);
        lcd.print(">");
        lcd.setCursor(12, 3);
        lcd.print(" ");
        lcd.setCursor(0, 3);
        lcd.print(" ");
        break;

      case 3:
        lcd.setCursor(12, 1);
        lcd.print(" ");
        lcd.setCursor(12, 2);
        lcd.print(" ");
        lcd.setCursor(12, 3);
        lcd.print(">");
        lcd.setCursor(0, 3);
        lcd.print(" ");
        break;

      case 4:
        lcd.setCursor(12, 1);
        lcd.print(" ");
        lcd.setCursor(12, 2);
        lcd.print(" ");
        lcd.setCursor(12, 3);
        lcd.print(" ");
        lcd.setCursor(0, 3);
        lcd.print(">");
        break;  
    }

    if (ret == UP){
      state--;
      if (state < 1)
        state = 4;
    }
    else if (ret == DOWN){
      state++;
      if (state > 4)
        state = 1;
    }
    else if (ret == ENTER){
      int dif, pin;
      int eThresh = readIntFromEEPROM(45);
      int mThresh = readIntFromEEPROM(55);
      int hThresh = readIntFromEEPROM(65);
      int thresh;
      if (state == 1){
        dif = 1;
        pin = _ePin;
        thresh = eThresh;
      }
      else if (state == 2){
        dif = 2;
        pin = _mPin;
        thresh = mThresh;
      }
      else if (state == 3){
        dif = 3;
        pin = _hPin;
        thresh = hThresh;
      }
      else{
        digitalWrite(_ePin, LOW);
        digitalWrite(_mPin, LOW);
        digitalWrite(_hPin, LOW);
        digitalWrite(_bypassPin, LOW);

        lcd.clear();
        lcd.print("Testing | Motors");
        lcd.setCursor(0,1);
        lcd.print("Difficulty: >easy  ");
        lcd.setCursor(0,2);
        lcd.print("             medium");
        lcd.setCursor(0,3);
        lcd.print(" back        hard  ");
        exit = true;
        continue;
      }

      lcd.clear();
      lcd.print("Testing | Motors");
      lcd.setCursor(0,1);
      lcd.print("Dif:");
      if (dif == 1)
        lcd.print("easy");
      else if (dif == 2)
        lcd.print("med");
      else
        lcd.print("hard");
      

      int warmupTime1 = 4000;
      int warmupTime2 = 6000;
      unsigned long warmupTimer1 = millis() + warmupTime1;
      unsigned long warmupTimer2 = millis() + warmupTime2;
      digitalWrite(_bypassPin, HIGH);
      delay(200);
      digitalWrite(pin, HIGH);

      lcd.setCursor(9,1);
      lcd.print("Thresh:");
      lcd.print(thresh);
      lcd.setCursor(0,2);
      lcd.print("Sensor value: warmup");
      lcd.setCursor(0,3);
      lcd.print(">edit");
      lcd.setCursor(7,3);
      lcd.print(" back");

      int state = 1;
      while(1){
        delay(50);
        if (millis() > warmupTimer1){
          digitalWrite(_bypassPin, LOW);
        }
        if (millis() > warmupTimer2){
          int val = analogRead(A8);

          lcd.setCursor(14,2);
          lcd.print("      ");
          lcd.setCursor(16,2);
          lcd.print(val);
        }

        int r = _buttonListener();
        if (r == UP || r == DOWN){
          if (state == 1){
            state = 2;
            lcd.setCursor(0,3);
            lcd.print(" ");
            lcd.setCursor(7,3);
            lcd.print(">");
          }
          else{
            state = 1;
            lcd.setCursor(0,3);
            lcd.print(">");
            lcd.setCursor(7,3);
            lcd.print(" ");
          }
        }

        if (r == ENTER){
          if (state == 2){
            digitalWrite(_ePin, LOW);
            digitalWrite(_mPin, LOW);
            digitalWrite(_hPin, LOW);
            digitalWrite(_bypassPin, LOW);

            lcd.clear();
            lcd.print("Testing | Motors");
            lcd.setCursor(0,1);
            lcd.print("Difficulty: >easy  ");
            lcd.setCursor(0,2);
            lcd.print("             medium");
            lcd.setCursor(0,3);
            lcd.print(" back        hard  ");
            break;
          }
          else{
            lcd.setCursor(8,1);
            lcd.print(">");
            delay(1000);
            while(1){
              delay(50);
              int val = _buttonListener();
              if (val == UP){
                thresh--;
                if (thresh < 0)
                  thresh = 0;

                lcd.setCursor(16,1);
                lcd.print("    ");
                lcd.setCursor(16,1);
                lcd.print(thresh);

                delay(400);
                while(digitalRead(_button[2]) == HIGH){
                  thresh--;
                  if (thresh < 0)
                    thresh = 0;

                  lcd.setCursor(16,1);
                  lcd.print("    ");
                  lcd.setCursor(16,1);
                  lcd.print(thresh);
                  delay(50);
                }
              }
              else if (val == DOWN){
                thresh++;
                if (thresh > 1023)
                  thresh = 1023;

                lcd.setCursor(16,1);
                lcd.print("    ");
                lcd.setCursor(16,1);
                lcd.print(thresh);

                delay(400);
                while(digitalRead(_button[3]) == HIGH){
                  thresh++;
                  if (thresh > 1023)
                    thresh = 1023;

                  lcd.setCursor(16,1);
                  lcd.print("    ");
                  lcd.setCursor(16,1);
                  lcd.print(thresh);
                  delay(50);
                }
              }
              else if (val == ENTER){
                if (dif == 1)
                  writeIntIntoEEPROM(45, thresh);
                else if (dif == 2)
                  writeIntIntoEEPROM(55, thresh);
                else if (dif == 3)
                  writeIntIntoEEPROM(65, thresh);

                lcd.setCursor(8,1);
                lcd.print(" ");
                break;
              }
            }
          }
        }
      }
    }
  }
}
void writeIntIntoEEPROM(int address, int number){ 
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

int readIntFromEEPROM(int address){
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}
