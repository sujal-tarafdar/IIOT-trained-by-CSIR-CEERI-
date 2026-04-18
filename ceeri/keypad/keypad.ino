#include <Keypad.h>

const byte ROWS = 4;   // four rows
const byte COLS = 4;   // four columns

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

// Connect to the row pinouts of the keypad
byte rowPins[ROWS] = {5, 18, 19, 21};

// Connect to the column pinouts of the keypad
byte colPins[COLS] = {0, 4, 16, 17};

// Create keypad object
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(115200);
}

void loop() {
  char customKey = customKeypad.getKey();

  if (customKey) {
    Serial.println(customKey);
  }
}