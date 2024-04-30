/*BASE BOARD - COM5
   GAME BOARD - COM6
   FINAL STATION - COM4
*/

#include <CubeRoomAgent.h>

char roomName[] = "LASER_MAZE";  // MySQL room name

//Room initialization
CubeRoomAgent room(roomName);

int score, timeRem;

unsigned long loopTimer;
const int loopDelay = 1000;

TimerEvent agentLoop;

unsigned long gameTime;

//----------------------------------------
//Laser Maze setup
const int lasers[] = {3, 4, 5, 6};
const int ldr[] = {A7, A8, A9, A11, A10, A12, A13};
const int diffLasers[] = {3, 4, 4};
const int diffLdr[] = {6, 7, 7};
const int laserThresh[] = {600, 600, 650, 850, 750, 650, 700};
const int smokePin = 14;
int smokeTime = 500;

const int thresh = 950;

unsigned long flashTiming = millis() + 1500;
unsigned long smokeTiming = 0;
bool lasersOn = true;
bool lasersActive = true;
unsigned long lasersActiveTiming;
boolean gameBegan;

//----------------------------------------
//Game setup
int diff = 0;
bool started = false;

void setup() {
  Serial.begin(9600);
  delay(2000);
  room.isBaseStation = false;
  agentLoop.set(1000, updateDataFromAgent);

  Serial.println("Game Board - COM6");

  gameBegan = true;

  pinMode(smokePin, OUTPUT);
  for (int i = 0; i < 4; i++) {
    pinMode(lasers[i], OUTPUT);
  }
  for (int i = 0; i < 7; i++) {
    pinMode(ldr[i], INPUT_PULLUP);
  }

  turnOff();
}

void updateDataFromAgent() {
  room.pingAgent();
}

void loop() {
  if (millis() > loopTimer) {   //check database loop
    room.updateData();
    loopTimer = millis() + loopDelay;

    if (room.isActive()) {
      int dif = room.getDifficulty();
      int numberOfPlayers = room.getNumberOfPlayers();
      int gameTime = room.getGameTime();
      while (room.getRoomStatusFromSerial() == room.activeStatus) {
        room.pingAgent();
        delay(1000);
      }
      if (room.getRoomStatusFromSerial() == room.activatedStatus) {

        int sc = startGame(dif, numberOfPlayers, gameTime);     //start the game, wait the game to finish

        if (sc == 0) {
          room.finishLose(timeRem);                             //lose, score = 0
        }
        else if (sc == -1) {
          room.finishTimeout();                                 //timeout, score = 0
        }
        else {
          room.finishWin(sc, timeRem);    //win and post score
        }
      }
    }
  }

  if (gameBegan) {
    turnOff();
    gameBegan = false;
  }

  delay(50);
}

int startGame(int difficulty, int numberOfPlayers, int gameTime) {
  Serial.println("Game Started");
  Serial.print("Game Time: ");
  Serial.println(gameTime);
  unsigned long timeoutTime = (gameTime * 1000L) + millis();
  unsigned long checkDbTime = millis() + 1000;

  gameBegan = true;
  digitalWrite(smokePin, HIGH);
  delay(2000);
  digitalWrite(smokePin, LOW);

  smokeTiming = millis() + smokeTime;


  diff = difficulty;
  for (int i = 0; i < 4; i++) {
    digitalWrite(lasers[i], HIGH);
  }
  delay(1000);

  while (millis() < timeoutTime) {
    if (millis() > checkDbTime) {
      checkDbTime = millis() + 1000;
      int rs = room.getRoomStatusFromSerial();
      if (rs == room.winStatus) {
        return 1;
        unsigned long timer = millis() + 20000;
        while (millis() < timer) {
          room.pingAgent();
          delay(50);
        }
      }
      else if (rs == room.loseStatus || rs == room.timeoutStatus) {
        return 0;
        unsigned long timer = millis() + 20000;
        while (millis() < timer) {
          room.pingAgent();
          delay(50);
        }
      }
      room.pingAgent();
    }

    if (millis() > smokeTiming) {
      digitalWrite(smokePin, LOW);
    }

    for (int i = 0; i < diffLasers[diff]; i++) {
      digitalWrite(lasers[i], HIGH);
    }
    for (int i = 0; i < diffLdr[diff]; i++) {
      int value = analogRead(ldr[i]);
      if (diff == 3 && lasersOn && (i == 2 || i == 3)) {
        value = 200;
      }
      if (value > laserThresh[i]) {
        return 0;
      }
    }

    if (diff == 3 && millis() > flashTiming) {
      lasersOn = !lasersOn;

      if (lasersOn) {
        digitalWrite(lasers[1], HIGH);
      }
      else {
        digitalWrite(lasers[1], LOW);
      }
    }
    room.pingAgent();
    delay(50);
  }
}

void turnOff() {
  digitalWrite(smokePin, LOW);
  for (int i = 0; i < 4; i++) {
    digitalWrite(lasers[i], LOW);
  }
}
