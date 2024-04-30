/*BASE BOARD - COM5
   GAME BOARD - COM6
   FINAL STATION - COM4
*/
#include <CubeRoomAgent.h>
#include <TimerOne.h>

char roomName[] = "LASER_MAZE";  // MySQL room name

//Room initialization
CubeRoomAgent room(roomName);

int score, timeRem;

unsigned long loopTimer;
const int loopDelay = 1000;
const int greenLightMillis = 10000;

const int diagnosisTestBtn = 47;

const int fogTime = 5000;
const long dissableFogTime = 300000;
//const long dissableFogTime = 10000;
#define FOG_MACHINE_PIN 9
#define FOG_BUTTON_PIN 43
unsigned long startFogTime;
unsigned long startDiasableTime;
bool fogMachineEnabled = 1;
bool isFogMachineOn = 0;

TimerEvent agentLoop;

//put games varieables here
const int rfidPins[] = { 18, 19, 23, 25, 27, 29 };
const int winScore[] = { 100, 300, 500 };
const int smokePin = A0;
void setup() {
  Serial.begin(9600);
  Timer1.attachInterrupt(isr, 100000); // Set the desired interval (in microseconds)
  pinMode(smokePin, OUTPUT);
  digitalWrite(smokePin, HIGH);
  pinMode(FOG_MACHINE_PIN, OUTPUT);
  Serial.println("Base Board - COM5");
  diagnosisSetup();
  agentLoop.set(1000, updateDataFromAgent);
  delay(3000);
}

void updateDataFromAgent() {
  room.pingAgent();
}

void loop() {
  if (millis() > loopTimer) {
    room.updateData();
    if (digitalRead(diagnosisTestBtn) == 1) {
      diagnosisStart();
      return;
    }
    loopTimer = millis() + loopDelay;

    if (room.isActive()) {
      int dif = room.getDifficulty();
      int numberOfPlayers = room.getNumberOfPlayers();
      int gameTime = room.getGameTime();

      bool runGame = room.waitToRun(1);  //1 - Game starts when door opens, 2 - Game starts when door opens and closes
      int score;
      if (runGame) {
        score = startGame(dif, numberOfPlayers, gameTime);  //start the game, wait the game to finish

        if (score == 0) {
          room.finishLose(timeRem);  //lose, score = 0
        } else if (score == -1) {
          room.finishTimeout();  //timeout, score = 0
        } else {
          room.finishWin(score, timeRem);  //win and post score
        }
      }
    }
    //room.disconnect();
    Serial.println();
  }

  if (room.checkEmergency()) {
    delay(5000);
    room.updateRoomStatus(room.inactiveStatus);
    return;
  }

  delay(50);
}

int startGame(int difficulty, int numberOfPlayers, int gameTime) {
  Serial.println("Game Started");
  unsigned long timeoutTime = (gameTime * 1000L) + millis();
  unsigned long greenLightTime = millis() + greenLightMillis;
  unsigned long greenChangeTime = millis() + 1000;
  unsigned long checkDbTime = millis() + 1000;
  bool green = false;

  while (millis() < timeoutTime) {
    if (millis() < greenLightTime) {
      if (millis() > greenChangeTime) {
        greenChangeTime = millis() + 1000;
        if (green)
          room.lightGreen();
        else
          room.lightOff();

        green = !green;
      }
    } else {
      room.lightCube();
      digitalWrite(5, LOW);
    }

    if (room.checkEmergency()) {
      timeRem = round((timeoutTime - millis()) / 1000);
      return 0;
    }

    if (millis() > greenLightTime && room.getDoorState() == 0) {
      Serial.println("Door");
      timeRem = round((timeoutTime - millis()) / 1000);
      return 0;
    }

    //put game code here
    if (millis() > checkDbTime) {
      checkDbTime = millis() + 1000;
      int rs = room.getRoomStatusFromSerial();
      if (rs == room.loseStatus) {
        return 0;
      }
    }

    int counter = 0;
    for (int i = 0; i < 6; i++) {
      if (digitalRead(rfidPins[i]) == 1) {
        counter++;
      }
    }
    if (counter == numberOfPlayers) {
      Serial.println("RFID");
      timeRem = round((timeoutTime - millis()) / 1000);
      return winScore[difficulty - 1];  //win
    }

    agentLoop.update();
    delay(50);
  }
  timeRem = 0;
  return -1;  //timeout
}

void isr() {
  if (fogMachineEnabled) {
    if (digitalRead(FOG_BUTTON_PIN)) {
      digitalWrite(FOG_MACHINE_PIN, 1);
      isFogMachineOn = 1;
      startDiasableTime = millis();
      startFogTime = millis();
      fogMachineEnabled = 0;
    }
  } else {
    if (isFogMachineOn) {
      if (millis() - startFogTime > fogTime) {
        digitalWrite(FOG_MACHINE_PIN, 0);
        isFogMachineOn = 0;
      }
    }
    if (millis() - startDiasableTime > dissableFogTime) {
      fogMachineEnabled = 1;
    }
  }
}
