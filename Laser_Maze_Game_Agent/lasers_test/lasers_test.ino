const int lasers[] = {3, 4, 5, 6};
const int ldr[] = {A0, A1, A2, A3, A4, A5, A6};
const int smokePin = 14;

const int diff = 2;
int smokeTime = 7000;

const int thresh = 950;

unsigned long flashTiming = millis() + 1500;
unsigned long smokeTiming = 0;
bool lasersOn = true;
bool lasersActive = true;
unsigned long lasersActiveTiming;

void setup() {
  Serial.begin(115200);
  pinMode(smokePin, OUTPUT);
  for (int i=0; i<4; i++){
    pinMode(lasers[i], OUTPUT);
    digitalWrite(lasers[i], HIGH);
  }
  for (int i=0; i<7; i++){
    pinMode(ldr[i], INPUT_PULLUP);
  }
  delay(1000);
  int c = 0;
  for (int i=0; i<7; i++){
    if (analogRead(ldr[i]) > 700){
      c++;
    }
  }
  if (c < 1){
    smokeTiming = millis() + smokeTime;
  }
  else{
    Serial.println("Enough smoke");
  }
}

void loop() {
  if (millis() < smokeTiming){
    digitalWrite(smokePin, HIGH);
  }
  else{
    digitalWrite(smokePin, LOW);
  }
  
  if (diff == 1){
    digitalWrite(lasers[0], HIGH);
    digitalWrite(lasers[1], LOW);
    digitalWrite(lasers[2], HIGH);
    digitalWrite(lasers[3], HIGH);
    for (int i=0; i<7; i++){
      int value = analogRead(ldr[i]);
      Serial.print(value);
      Serial.print("\t");
      if ((i == 1 || i == 2 || i == 3 || i == 5 || i == 6) && value > thresh){
        lose();
      }
    }
    Serial.println();
  }
  else if (diff == 2){
    digitalWrite(lasers[0], HIGH);
    digitalWrite(lasers[1], HIGH);
    digitalWrite(lasers[2], HIGH);
    digitalWrite(lasers[3], HIGH);
    for (int i=0; i<7; i++){
      int value = analogRead(ldr[i]);
      Serial.print(value);
      Serial.print("\t");
      if (value > thresh){
        lose();
      }
    }
    Serial.println();
  }
  else if (diff == 3){
    digitalWrite(lasers[0], HIGH);
    digitalWrite(lasers[2], HIGH);
    if (millis() > flashTiming){
      flashTiming = millis() + 1500;
      lasersActiveTiming = millis() + 300;
      lasersOn = !lasersOn;
      
      if (lasersOn){
        digitalWrite(lasers[1], HIGH);
        digitalWrite(lasers[3], HIGH);
      }
      else{
        digitalWrite(lasers[1], LOW);
        digitalWrite(lasers[3], LOW);
      }
    }

    for (int i=0; i<7; i++){
      int value = analogRead(ldr[i]);
      Serial.print(value);
      Serial.print("\t");
      if (!lasersOn){
        if ((i == 1 || i == 2 || i == 5 || i == 6) && value > thresh)
          lose();
      }
      else{
        if (value > thresh && millis() > lasersActiveTiming){
          lose();
        }
      }
    }
    Serial.println();
  }

  delay(50);
}

void lose(){
  for (int i=0; i<4; i++){
    digitalWrite(lasers[i], LOW);
  }
  delay(5000);
  for (int i=0; i<4; i++){
    digitalWrite(lasers[i], HIGH);
  }
  delay(500);
//Serial.println("lose");
}
