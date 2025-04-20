#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD Configuration
LiquidCrystal_I2C lcd(0x26, 16, 2);
LiquidCrystal_I2C lcdTIME(0x3F, 16, 2);

// Buzzer Pins
const int timerBuzzerPin = 22;
const int gameBuzzerPin = 7;

struct Note {
  unsigned int frequency;
  unsigned long duration;
};

// Musical Notes
const unsigned int C6 = 1047;
const unsigned int Ab5 = 831;
const unsigned int Bb5 = 932;
const unsigned int G5 = 784;
const unsigned int F5 = 698;
const unsigned int E5 = 659;
const unsigned int Eb5 = 622;
const unsigned int D5 = 587;
const unsigned int Db5 = 554;
const unsigned int C5 = 523;
const unsigned int B4 = 494;
const unsigned int Bb4 = 466;
const unsigned int nA4 = 440;
const unsigned int Ab4 = 415;
const unsigned int G4 = 392;
const unsigned int Gb4 = 370;
const unsigned int F4 = 349;
const unsigned int E4 = 330;
const unsigned int Eb4 = 311;
const unsigned int D4 = 294;
const unsigned int Db4 = 277;
const unsigned int C4 = 262;
const unsigned int B3 = 247;
const unsigned int Bb3 = 233;
const unsigned int nA3 = 220;
const unsigned int G3 = 196;
const unsigned int Gb3 = 185;
const unsigned int F3 = 175;
const unsigned int E3 = 165;

enum SoundState { SOUND_IDLE, SOUND_PLAYING, SOUND_PAUSE };
SoundState soundState = SOUND_IDLE;

enum TimerBeepState { TIMER_IDLE, TIMER_PLAYING, TIMER_PAUSE };
TimerBeepState timerBeepState = TIMER_IDLE;

// Beeps
Note beep[] = {{1000, 50}};
Note fastBeep[] = {{1000, 50}};
Note fasterBeep[] = {{1000, 50}};
Note fastestBeep[] = {{1000, 50}};

// Game Sounds
Note successMelody[] = {
  {G3, 166}, {C4, 166}, {E4, 166}, {G4, 166}, {C5, 166}, {E5, 166}, {G5, 500}, {E5, 500},
  {E3, 166}, {C4, 166}, {Eb4, 166}, {Ab4, 166}, {C5, 166}, {Eb5, 166}, {Ab5, 500}, {Eb5, 500},
  {Bb3, 166}, {D4, 166}, {F4, 166}, {Bb4, 166}, {D5, 166}, {F5, 166}, {Bb5, 500},
  {0, 50}, {Bb5, 166}, {0, 50}, {Bb5, 166}, {0, 50}, {Bb5, 166}, {C6, 2000}
};

Note correctSound[] = {
  {D5, 110}, {0, 40}, {G5, 150}
};

Note wrongSound[] = {
  {Eb4, 300}, {0, 50}, {E3, 500},
  {200, 150}, {190, 150}, {180, 150}, {170, 150}, {160, 150},
  {150, 150}, {140, 150}, {130, 150}, {120, 150}, {110, 150},
  {100, 150}, {90, 150}, {80, 150}, {70, 150}, {60, 150}, {50, 150}
};

Note* currentMelody = nullptr;
int currentNoteIndex = 0;
int melodyLength = 0;
unsigned long soundStartTime;
unsigned long currentNoteDuration;

Note* currentTimerMelody = nullptr;
int currentTimerNoteIndex = 0;
int timerMelodyLength = 0;
unsigned long timerSoundStartTime;
unsigned long currentTimerNoteDuration;

bool timerBeepActive = false;
unsigned long timerBeepEndTime = 0;
unsigned long lastBeepTime = 0;

// Game Hardware
const int redJumperPin = 8;
const int orangeJumperPin = 9;
const int yellowJumperPin = 10;
const int greenJumperPin = 11;
const int blueJumperPin = 12;
const int purpleJumperPin = 13;
const int jumperPins[] = {redJumperPin, orangeJumperPin, yellowJumperPin, greenJumperPin, blueJumperPin, purpleJumperPin};
const int numPins = sizeof(jumperPins) / sizeof(jumperPins[0]);
const int sequenceLength = numPins;
const int offsetPin = -7;
const unsigned long TIME_LIMIT_MS = 5 * 60 * 1000UL;

// Timing
const int FAST_PERCENT = 10;
const int FASTER_PERCENT = 5;
const int FASTEST_PERCENT = 2;
const unsigned long BEEP_INTERVALS[] = {1000, 500, 250, 125};
unsigned long FAST_THRESHOLD_MS;
unsigned long FASTER_THRESHOLD_MS;
unsigned long FASTEST_THRESHOLD_MS;

// LEDs
const int whiteMasterPin = 2;
const int greenPin = 3;
const int redPin = 4;
const int bluePin = 5;
const int yellowPin = 6;
const int ledPins[] = {greenPin, redPin, bluePin, yellowPin};
const int numLeds = sizeof(ledPins) / sizeof(ledPins[0]);

// LED patterns (G,R,B,Y order)
const bool ledPatterns[16][4] = {
  {0,0,0,0}, //0
  {0,0,0,1}, //1
  {0,0,1,0}, //2
  {0,0,1,1}, //3
  {0,1,0,0}, //4
  {0,1,0,1}, //5
  {0,1,1,0}, //6
  {0,1,1,1}, //7
  {1,0,0,0}, //8
  {1,0,0,1}, //9
  {1,0,1,0}, //10
  {1,0,1,1}, //11
  {1,1,0,0}, //12
  {1,1,0,1}, //13
  {1,1,1,0}, //14
  {1,1,1,1}  //15
};

// ===== SEQUENCES (INDEX-BASED) =====
const int sequence0[] = {1, 0, 4, 2, 3, 5};
const int sequence1[] = {5, 3, 2, 4, 1, 0};
const int sequence2[] = {3, 4, 2, 1, 5, 0};
const int sequence3[] = {4, 5, 2, 0, 3, 1};
const int sequence4[] = {4, 3, 1, 5, 2, 0};
const int sequence5[] = {0, 4, 1, 2, 3, 5};
const int sequence6[] = {5, 3, 1, 2, 0, 4};
const int sequence7[] = {2, 1, 3, 4, 5, 0};
const int sequence8[] = {4, 1, 3, 5, 0, 2};
const int sequence9[] = {0, 4, 3, 2, 5, 1};
const int sequence10[] = {0, 2, 3, 4, 1, 5};
const int sequence11[] = {2, 3, 5, 1, 4, 0};
const int sequence12[] = {4, 5, 1, 3, 2, 0};
const int sequence13[] = {2, 0, 5, 1, 3, 4};
const int sequence14[] = {1, 0, 2, 5, 4, 3};
const int sequence15[] = {1, 3, 0, 5, 2, 4};

const int* sequences[] = {
  sequence0, sequence1, sequence2, sequence3, sequence4,
  sequence5, sequence6, sequence7, sequence8, sequence9,
  sequence10, sequence11, sequence12, sequence13, sequence14, sequence15
};
const int numSequences = sizeof(sequences) / sizeof(sequences[0]);

// Game State
int currentSequence[numPins];
int currentStep = 0;
bool pinDisconnected[numPins] = {false};
bool waitingForReconnect = true;
bool timeExpired = false;
unsigned long gameStartTime = 0;
unsigned long lastPrintTime = 0;
int lastDisplayedSecond = -1;
int currentSequenceIndex = 0;
const bool printDebug = true;
const bool lcdOn = true;

void playMelody(Note* melody, int length, bool isTimerBeep) {
  if (isTimerBeep) {
    currentTimerMelody = melody;
    timerMelodyLength = length;
    currentTimerNoteIndex = 0;
    timerBeepState = TIMER_PLAYING;
    playCurrentTimerNote();
  } else {
    currentMelody = melody;
    melodyLength = length;
    currentNoteIndex = 0;
    soundState = SOUND_PLAYING;
    playCurrentNote(false);
  }
}

void playCurrentNote(bool isTimerBeep) {
  if (currentNoteIndex >= melodyLength) {
    soundState = SOUND_IDLE;
    noTone(gameBuzzerPin);
    return;
  }

  Note note = currentMelody[currentNoteIndex];
  if (note.frequency > 0) {
    tone(gameBuzzerPin, note.frequency);
  } else {
    noTone(gameBuzzerPin);
  }
  
  currentNoteDuration = note.duration;
  soundStartTime = millis();
  currentNoteIndex++;
}

void playCurrentTimerNote() {
  if (currentTimerNoteIndex >= timerMelodyLength) {
    timerBeepState = TIMER_IDLE;
    noTone(timerBuzzerPin);
    return;
  }

  Note note = currentTimerMelody[currentTimerNoteIndex];
  if (note.frequency > 0) {
    tone(timerBuzzerPin, note.frequency);
  } else {
    noTone(timerBuzzerPin);
  }
  
  currentTimerNoteDuration = note.duration;
  timerSoundStartTime = millis();
  currentTimerNoteIndex++;
}

void updateSound() {
  // Handle game sounds
  if (soundState != SOUND_IDLE) {
    unsigned long currentTime = millis();
    
    if (soundState == SOUND_PLAYING && (currentTime - soundStartTime) >= currentNoteDuration) {
      noTone(gameBuzzerPin);
      soundState = SOUND_PAUSE;
      soundStartTime = currentTime;
      currentNoteDuration = max(20, currentNoteDuration * 0.1);
    }
    else if (soundState == SOUND_PAUSE && (currentTime - soundStartTime) >= currentNoteDuration) {
      playCurrentNote(false);
    }
  }

  // Handle timer beeps
  if (timerBeepState != TIMER_IDLE) {
    unsigned long currentTime = millis();
    
    if (timerBeepState == TIMER_PLAYING && (currentTime - timerSoundStartTime) >= currentTimerNoteDuration) {
      noTone(timerBuzzerPin);
      timerBeepState = TIMER_PAUSE;
      timerSoundStartTime = currentTime;
      currentTimerNoteDuration = max(20, currentTimerNoteDuration * 0.1);
    }
    else if (timerBeepState == TIMER_PAUSE && (currentTime - timerSoundStartTime) >= currentTimerNoteDuration) {
      playCurrentTimerNote();
    }
  }
}

void sucesso() {
  playMelody(successMelody, sizeof(successMelody)/sizeof(successMelody[0]), false);
}

void correto() {
  playMelody(correctSound, sizeof(correctSound)/sizeof(correctSound[0]), false);
}

void errado() {
  playMelody(wrongSound, sizeof(wrongSound)/sizeof(wrongSound[0]), false);
}

void playTimerBeep() {
  tone(timerBuzzerPin, beep[0].frequency);
  timerBeepEndTime = millis() + beep[0].duration;
  timerBeepActive = true;
}

void playFastTimerBeep() {
  tone(timerBuzzerPin, fastBeep[0].frequency);
  timerBeepEndTime = millis() + fastBeep[0].duration;
  timerBeepActive = true;
}

void playFasterTimerBeep() {
  tone(timerBuzzerPin, fasterBeep[0].frequency);
  timerBeepEndTime = millis() + fasterBeep[0].duration;
  timerBeepActive = true;
}

void playFastestTimerBeep() {
  tone(timerBuzzerPin, fastestBeep[0].frequency);
  timerBeepEndTime = millis() + fastestBeep[0].duration;
  timerBeepActive = true;
}

void updateTimerBeep() {
  if (timerBeepActive && millis() >= timerBeepEndTime) {
    noTone(timerBuzzerPin);
    timerBeepActive = false;
  }
}

void resetLeds() {
  for (int i = 0; i < numLeds; i++) {
    digitalWrite(ledPins[i], LOW);
  }
}

void indicateLed(int seqNumber) {
  resetLeds();
  if (seqNumber >= 0 && seqNumber < 16) {
    for (int i = 0; i < numLeds; i++) {
      if (ledPatterns[seqNumber][i] == 1) {
        digitalWrite(ledPins[i], HIGH);
      }
    }
  }
}

int getDisplayPin(int actualPin) {
  return actualPin + offsetPin;
}

String getPinString(int actualPin) {
  return String(getDisplayPin(actualPin));
}

String getSequenceString() {
  String result;
  for (int i = 0; i < sequenceLength; i++) {
    if (i > 0) result += ",";
    result += getPinString(jumperPins[currentSequence[i]]);
  }
  return result;
}

void selectNewSequence() {
  currentSequenceIndex = random(numSequences);
  for (int i = 0; i < sequenceLength; i++) {
    currentSequence[i] = sequences[currentSequenceIndex][i];
  }
  indicateLed(currentSequenceIndex);

  if (printDebug) {
    Serial.print("Selected sequence (");
    Serial.print(currentSequenceIndex);
    Serial.print("): ");
    Serial.println(getSequenceString());
  }
}

void displayLCD(const String& line1, const String& line2 = "") {
  if (!lcdOn) return;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  if (line2 != "") {
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
}

void displayTimeLCD(const String& line1, const String& line2 = "") {
  if (!lcdOn) return;
  lcdTIME.clear();
  lcdTIME.setCursor(0, 0);
  lcdTIME.print(line1);
  if (line2 != "") {
    lcdTIME.setCursor(0, 1);
    lcdTIME.print(line2);
  }
}

String formatTime(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  int minutes = seconds / 60;
  int remainingSeconds = seconds % 60;
  char buffer[6];
  sprintf(buffer, "%02d:%02d", minutes, remainingSeconds);
  return String(buffer);
}

void displayTimeRemaining(unsigned long remainingMs) {
  displayTimeLCD("Tempo restante:", formatTime(remainingMs));
}

void updateTimeDisplay() {
  unsigned long remaining = TIME_LIMIT_MS - (millis() - gameStartTime);
  int currentSecond = remaining / 1000;
  
  if (currentSecond != lastDisplayedSecond) {
    lastDisplayedSecond = currentSecond;
    displayTimeRemaining(remaining);
  }

  if (!timeExpired && !waitingForReconnect) {
    int beepMode;
    if (remaining <= FASTEST_THRESHOLD_MS) {
      beepMode = 3;  // Fastest beep
    } else if (remaining <= FASTER_THRESHOLD_MS) {
      beepMode = 2;  // Faster beep
    } else if (remaining <= FAST_THRESHOLD_MS) {
      beepMode = 1;  // Fast beep
    } else {
      beepMode = 0;  // Normal beep
    }

    if (millis() - lastBeepTime >= BEEP_INTERVALS[beepMode]) {
      switch (beepMode) {
        case 3: playFastestTimerBeep(); break;
        case 2: playFasterTimerBeep(); break;
        case 1: playFastTimerBeep(); break;
        default: playTimerBeep(); break;
      }
      lastBeepTime = millis();
    }
  }
}

void handleCorrectDisconnection(int pin) {
  if (printDebug) {
    Serial.println("V Correct!");
    Serial.print("Display Pin: ");
    Serial.println(getDisplayPin(pin));
    Serial.print("Actual Pin: ");
    Serial.println(pin);
  }
  if (lcdOn) {
    displayLCD("Correto!", "Continue!");
  }
  correto();

  if (++currentStep == sequenceLength) {
    unsigned long completionTime = millis() - gameStartTime;
    if (printDebug) {
      Serial.println("SUCCESS!");
      Serial.print("Time: ");
      Serial.println(formatTime(completionTime));
    }
    if (lcdOn) {
      displayLCD("SUCCESSO!", "Sequencia completa");
      displayTimeLCD("Completo em:", formatTime(completionTime));
    }
    sucesso();
    waitingForReconnect = true;
  }
}

void handleWrongDisconnection(int actual, int expected) {
  if (printDebug) {
    Serial.print("X Expected ");
    Serial.print("Display Pin: ");
    Serial.println(getDisplayPin(expected));
    Serial.print("Actual Pin: ");
    Serial.println(expected);
  }
  if (lcdOn) {
    displayLCD("Incorreto!", "BOOM!");
  }
  errado();
  waitingForReconnect = true;
}

bool allJumpersConnected() {
  for (int i = 0; i < numPins; i++) {
    if (digitalRead(jumperPins[i]) == HIGH) return false;
  }
  return true;
}

void resetForNewAttempt() {
  // Stop all sounds immediately
  noTone(gameBuzzerPin);
  noTone(timerBuzzerPin);
  
  // Reset game sound state
  soundState = SOUND_IDLE;
  currentMelody = nullptr;
  currentNoteIndex = 0;
  
  // Reset timer beep state
  timerBeepState = TIMER_IDLE;
  currentTimerMelody = nullptr;
  currentTimerNoteIndex = 0;
  timerBeepActive = false;
  
  // Reset game state
  currentStep = 0;
  memset(pinDisconnected, false, sizeof(pinDisconnected));
  
  // Reset any pending beeps
  lastBeepTime = millis(); // This prevents immediate beep after reset
}

void remindReconnect() {
  if (millis() - lastPrintTime > 2000) {
    if (printDebug) Serial.println("Please connect ALL jumpers...");
    if (lcdOn) {
      displayLCD("Por favor", "reconecte tudo");
      displayTimeLCD("Aguardando", "reconexao...");
    }
    lastPrintTime = millis();
  }
}

void checkForDisconnections() {
  int expectedPinIndex = currentSequence[currentStep];
  int expectedPin = jumperPins[expectedPinIndex];

  for (int i = 0; i < numPins; i++) {
    int currentPin = jumperPins[i];
    bool isDisconnected = digitalRead(currentPin);
    
    if (isDisconnected && !pinDisconnected[i]) {
      pinDisconnected[i] = true;
      if (currentPin == expectedPin) {
        handleCorrectDisconnection(expectedPin);
      } else {
        handleWrongDisconnection(currentPin, expectedPin);
      }
    }
  }
}

void setup() {
  FAST_THRESHOLD_MS = TIME_LIMIT_MS * FAST_PERCENT / 100;
  FASTER_THRESHOLD_MS = TIME_LIMIT_MS * FASTER_PERCENT / 100;
  FASTEST_THRESHOLD_MS = TIME_LIMIT_MS * FASTEST_PERCENT / 100;

  soundState = SOUND_IDLE;
  timerBeepState = TIMER_IDLE;
  timerBeepActive = false;
  currentMelody = nullptr;
  currentTimerMelody = nullptr;

  pinMode(timerBuzzerPin, OUTPUT);
  pinMode(gameBuzzerPin, OUTPUT);
  digitalWrite(timerBuzzerPin, LOW);
  digitalWrite(gameBuzzerPin, LOW);
  
  pinMode(whiteMasterPin, OUTPUT);
  digitalWrite(whiteMasterPin, HIGH);
  
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  resetLeds();

  for (int i = 0; i < numPins; i++) {
    pinMode(jumperPins[i], INPUT_PULLUP);
  }

  if (printDebug) {
    Serial.begin(9600);
    Serial.println("System ready");
  }

  if (lcdOn) {
    lcd.init();
    lcd.backlight();
    lcdTIME.init();
    lcdTIME.backlight();
    displayLCD("Inicializando...");
    displayTimeLCD("Inicializando...");
  }

  randomSeed(analogRead(0));
}

void loop() {
  updateSound();
  updateTimerBeep();
  
  static bool timeUpMessageShown = false;
  
  if (timeExpired) {
    if (!timeUpMessageShown) {
      displayLCD("ACABOU O TEMPO!", "Reconecte tudo");
      displayTimeLCD("ACABOU O TEMPO!", "Reconecte tudo");
      errado();
      delay(1000);
      remindReconnect();
      timeUpMessageShown = true;
    }
    if (allJumpersConnected()) {
      waitingForReconnect = true;
      timeExpired = false;
      timeUpMessageShown = false;
    }
    return;
  }

  if (waitingForReconnect) {
    if (allJumpersConnected()) {
      waitingForReconnect = false;
      resetForNewAttempt();
      selectNewSequence();
      gameStartTime = millis();
      lastDisplayedSecond = -1;
      displayLCD("Comece!");
      displayTimeRemaining(TIME_LIMIT_MS);
    } else {
      remindReconnect();
    }
    return;
  }

  if (millis() - gameStartTime > TIME_LIMIT_MS) {
    timeExpired = true;
    return;
  }

  updateTimeDisplay();
  checkForDisconnections();
}
