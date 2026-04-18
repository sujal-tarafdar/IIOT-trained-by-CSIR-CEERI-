#include <Keypad.h>
#include <LiquidCrystal.h>
#include <string.h>

#define LED1 13
#define LED2 15

// LCD pins
LiquidCrystal lcd(14, 27, 26, 25, 33, 32);

// Keypad setup
const byte ROWS = 4;
const byte COLS = 4;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {19, 4, 16, 17};
byte colPins[COLS] = {5, 18, 23, 21};

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Password
char correctPassword[] = "1234";
char enteredPassword[5];

int passIndex = 0;   // ✅ renamed (IMPORTANT)
int attempts = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
  lcd.begin(20, 4);

  lcd.print("Enter Password:");
  lcd.setCursor(0, 1);
}

void loop() {
  char key = customKeypad.getKey();

  if (key) {
    Serial.println(key);

    // CLEAR
    if (key == '*') {
      passIndex = 0;
      lcd.clear();
      lcd.print("Enter Password:");
      lcd.setCursor(0, 1);
      return;
    }

    // ENTER
    if (key == '#') {

      enteredPassword[passIndex] = '\0';

      if (strcmp(enteredPassword, correctPassword) == 0) {
        lcd.clear();
        lcd.print("Password Correct");
        delay(1500);

        lcd.clear();
        digitalWrite(LED1,HIGH);
        lcd.print("Access Granted");
        delay(1500);
         digitalWrite(LED1,LOW);

        attempts = 0;
      }
      else {
        attempts++;

        lcd.clear();
        lcd.print("Wrong Password");
        digitalWrite(LED2,HIGH);
        delay(1500);
         digitalWrite(LED2,LOW);

        if (attempts >= 3) {
          lcd.clear();
          lcd.print("SYSTEM LOCKED");
          lcd.setCursor(0, 1);
          lcd.print("Wait 10 sec");
           digitalWrite(LED2,HIGH);
          delay(10000);
           digitalWrite(LED2,LOW);
          attempts = 0;
        }
      }

      passIndex = 0;
      lcd.clear();
      lcd.print("Enter Password:");
      lcd.setCursor(0, 1);

      return;
    }

    // STORE INPUT
    if (passIndex < 4) {
      enteredPassword[passIndex++] = key;
      lcd.print('*');
    }
  }
}