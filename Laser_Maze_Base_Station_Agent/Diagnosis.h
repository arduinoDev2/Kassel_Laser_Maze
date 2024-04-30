#ifndef Diagnosis_h
#define Diagnosis_h

#include "Arduino.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include <EEPROM.h>

#define MENU_ENTRIES 5

void diagnosisSetup();
void diagnosisStart();

int buttonListener();

void checkConnection();
void checkRGBLedTape();
void checkDoorLock();
void checkDoorState();
void checkBoardRelays();

void lightRed();
void lightGreen();
void lightBlue();
void lightCube();
void lightOff();

#endif
