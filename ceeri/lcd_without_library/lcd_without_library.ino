#define RS_PIN 16   // Register Select
#define EN_PIN 17   // Enable
#define D4_PIN 18   // Data Line 4
#define D5_PIN 19   // Data Line 5
#define D6_PIN 21   // Data Line 6
#define D7_PIN 22   // Data Line 7

void setup() {
  pinMode(RS_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  pinMode(D4_PIN, OUTPUT);
  pinMode(D5_PIN, OUTPUT);
  pinMode(D6_PIN, OUTPUT);
  pinMode(D7_PIN, OUTPUT);

  lcdInit();
}

void loop() {
  lcdClear();
  lcdPrint("Hello, World!");
  delay(2000);
}

// Send command to LCD
void lcdCommand(unsigned char cmd) {
  digitalWrite(RS_PIN, LOW);   // Command mode

  // Higher nibble
  digitalWrite(D4_PIN, (cmd >> 4) & 1);
  digitalWrite(D5_PIN, (cmd >> 5) & 1);
  digitalWrite(D6_PIN, (cmd >> 6) & 1);
  digitalWrite(D7_PIN, (cmd >> 7) & 1);

  digitalWrite(EN_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(EN_PIN, LOW);
  delayMicroseconds(100);

  // Lower nibble
  digitalWrite(D4_PIN, cmd & 1);
  digitalWrite(D5_PIN, (cmd >> 1) & 1);
  digitalWrite(D6_PIN, (cmd >> 2) & 1);
  digitalWrite(D7_PIN, (cmd >> 3) & 1);

  digitalWrite(EN_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(EN_PIN, LOW);
  delay(2);
}

// Send data (character) to LCD
void lcdData(unsigned char data) {
  digitalWrite(RS_PIN, HIGH);   // Data mode

  // Higher nibble
  digitalWrite(D4_PIN, (data >> 4) & 1);
  digitalWrite(D5_PIN, (data >> 5) & 1);
  digitalWrite(D6_PIN, (data >> 6) & 1);
  digitalWrite(D7_PIN, (data >> 7) & 1);

  digitalWrite(EN_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(EN_PIN, LOW);
  delayMicroseconds(100);

  // Lower nibble
  digitalWrite(D4_PIN, data & 1);
  digitalWrite(D5_PIN, (data >> 1) & 1);
  digitalWrite(D6_PIN, (data >> 2) & 1);
  digitalWrite(D7_PIN, (data >> 3) & 1);

  digitalWrite(EN_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(EN_PIN, LOW);
  delay(2);
}

// LCD initialization
void lcdInit() {
  delay(50);
  lcdCommand(0x02);   // 4-bit mode initialization
  lcdCommand(0x28);   // 4-bit mode, 2 lines, 5x7 format
  lcdCommand(0x0C);   // Display ON, cursor OFF, blinking OFF
  lcdCommand(0x06);   // Entry mode: increment cursor, no shift
  lcdClear();
}

// Clear LCD
void lcdClear() {
  lcdCommand(0x01);
  delay(2);
}

// Print string
void lcdPrint(const char *str) {
  while (*str) {
    lcdData(*str);
    str++;
  }
}