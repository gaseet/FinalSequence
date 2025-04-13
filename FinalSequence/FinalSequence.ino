#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===== LCD CONFIGURATION =====
LiquidCrystal_I2C lcd(0x27, 16, 2);      // Main game display
LiquidCrystal_I2C lcdTIME(0x26, 16, 2);  // Time display only

// ===== HARDWARE CONFIGURATION =====
// MUST FOLLOW ORDER: RED, ORANGE, YELLOW, GREEN, BLUE, AND PURPLE
// The jumper wires should be connected to the physical pins (or any others, 
// as long as they are properly adjusted in the code) in this exact order:
//
// 1. RED
// 2. ORANGE
// 3. YELLOW
// 4. GREEN
// 5. BLUE
// 6. PURPLE
//
// For example:
// Pin 8 -> RED
// Pin 9 -> ORANGE
// Pin 10 -> YELLOW
// Pin 11 -> GREEN
// Pin 12 -> BLUE
// Pin 13 -> PURPLE

// Define the pins for each color (jumper pins)
const int redJumperPin = 8;
const int orangeJumperPin = 9;
const int yellowJumperPin = 10;
const int greenJumperPin = 11;
const int blueJumperPin = 12;
const int purpleJumperPin = 13;

// Array of jumper pins in the same order as the colors
const int jumperPins[] = {redJumperPin, orangeJumperPin, yellowJumperPin, greenJumperPin, blueJumperPin, purpleJumperPin};  // Physical pins in order


const int numPins = sizeof(jumperPins) / sizeof(jumperPins[0]);
const int sequenceLength = numPins;
const int offsetPin = -7;  // Debug offset (pin 8 → "1")
const unsigned long TIME_LIMIT_MS = 5 * 60 * 1000UL;  // 5 minutes

// ===== LED CONFIGURATION =====
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
// Each value is an index into jumperPins (0-5)
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

// ===== RUNTIME VARIABLES =====
int currentSequence[numPins];  // Stores indices (0-5) of jumperPins
int currentStep = 0;
bool pinDisconnected[numPins] = {false};
bool waitingForReconnect = true;
bool timeExpired = false;
unsigned long gameStartTime = 0;
unsigned long lastPrintTime = 0;
int lastDisplayedSecond = -1;
int currentSequenceIndex = 0;

// Output control
const bool printDebug = true;
const bool lcdOn = true;

// ==== LED FUNCTIONS ====
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

// ===== PIN DISPLAY FUNCTIONS =====
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
    result += getPinString(jumperPins[currentSequence[i]]);  // Resolve index to pin
  }
  return result;
}

// ===== SEQUENCE SELECTION =====
void selectNewSequence() {
  currentSequenceIndex = random(numSequences);
  //currentSequenceIndex = 3;
  for (int i = 0; i < sequenceLength; i++) {
    currentSequence[i] = sequences[currentSequenceIndex][i];  // Store indices (0-5)
  }
  indicateLed(currentSequenceIndex);

  if (printDebug) {
    Serial.print("Selected sequence (");
    Serial.print(currentSequenceIndex);
    Serial.print("): ");
    Serial.println(getSequenceString());
  }
}

// ===== LCD DISPLAY FUNCTIONS =====
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

// ===== TIME FUNCTIONS =====
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
}

// ===== GAME FUNCTIONS =====
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
    delay(1000);
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
  delay(1000);
  waitingForReconnect = true;
}

bool allJumpersConnected() {
  for (int i = 0; i < numPins; i++) {
    if (digitalRead(jumperPins[i]) == HIGH) return false;
  }
  return true;
}

void resetForNewAttempt() {
  currentStep = 0;
  memset(pinDisconnected, false, sizeof(pinDisconnected));
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
  delay(100);
}

void checkForDisconnections() {
  int expectedPinIndex = currentSequence[currentStep];  // Get index (0-5)
  int expectedPin = jumperPins[expectedPinIndex];      // Resolve actual pin

  for (int i = 0; i < numPins; i++) {
    int currentPin = jumperPins[i];
    bool isDisconnected = digitalRead(currentPin) == HIGH;
    
    if (isDisconnected && !pinDisconnected[i]) {
      pinDisconnected[i] = true;
      if (currentPin == expectedPin) {
        handleCorrectDisconnection(expectedPin);
      } else {
        handleWrongDisconnection(currentPin, expectedPin);
      }
      delay(1);  // Minimal debounce
    }
  }
}

void setup() {
  // Initialize LED pins
  pinMode(whiteMasterPin, OUTPUT);
  digitalWrite(whiteMasterPin, HIGH);
  for (int i = 0; i < numLeds; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  resetLeds();

  // Initialize jumper pins
  for (int i = 0; i < numPins; i++) {
    pinMode(jumperPins[i], INPUT_PULLUP);
  }

  // Initialize serial
  if (printDebug) {
    Serial.begin(9600);
    Serial.println("System ready");
  }

  // Initialize LCDs
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
  static bool timeUpMessageShown = false;
  
  if (timeExpired) {
    if (!timeUpMessageShown) {
      displayLCD("ACABOU O TEMPO!", "Reconecte tudo");
      displayTimeLCD("ACABOU O TEMPO!", "Reconecte tudo");
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
