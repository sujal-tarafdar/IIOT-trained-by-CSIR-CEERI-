// ================= PIN CONFIG =================
int R1 = 32, R2 = 33, R3 = 25, R4 = 26;
int C1 = 27, C2 = 14, C3 = 12, C4 = 13;
char key = ' ';

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(R1, INPUT_PULLUP);
  pinMode(R2, INPUT_PULLUP);
  pinMode(R3, INPUT_PULLUP);
  pinMode(R4, INPUT_PULLUP);

  pinMode(C1, OUTPUT);
  pinMode(C2, OUTPUT);
  pinMode(C3, OUTPUT);
  pinMode(C4, OUTPUT);
}

// ================= LOOP =================
void loop() {
  key = 'F';

  while (key == 'F') {
    key = ReadKeypad();
  }

  Serial.println(key);
}

// ================= FUNCTION =================
char ReadKeypad() {
  char read_key = 'F';

  // -------- COLUMN 1 --------
  digitalWrite(C1, LOW);
  digitalWrite(C2, HIGH);
  digitalWrite(C3, HIGH);
  digitalWrite(C4, HIGH);

  if (digitalRead(R1) == LOW) { read_key = '1'; delay(300); }
  if (digitalRead(R2) == LOW) { read_key = '4'; delay(300); }
  if (digitalRead(R3) == LOW) { read_key = '7'; delay(300); }
  if (digitalRead(R4) == LOW) { read_key = '*'; delay(300); }

  // -------- COLUMN 2 --------
  digitalWrite(C1, HIGH);
  digitalWrite(C2, LOW);
  digitalWrite(C3, HIGH);
  digitalWrite(C4, HIGH);

  if (digitalRead(R1) == LOW) { read_key = '2'; delay(300); }
  if (digitalRead(R2) == LOW) { read_key = '5'; delay(300); }
  if (digitalRead(R3) == LOW) { read_key = '8'; delay(300); }
  if (digitalRead(R4) == LOW) { read_key = '0'; delay(300); }

  // -------- COLUMN 3 --------
  digitalWrite(C1, HIGH);
  digitalWrite(C2, HIGH);
  digitalWrite(C3, LOW);
  digitalWrite(C4, HIGH);

  if (digitalRead(R1) == LOW) { read_key = '3'; delay(300); }
  if (digitalRead(R2) == LOW) { read_key = '6'; delay(300); }
  if (digitalRead(R3) == LOW) { read_key = '9'; delay(300); }
  if (digitalRead(R4) == LOW) { read_key = '#'; delay(300); }

  // -------- COLUMN 4 --------
  digitalWrite(C1, HIGH);
  digitalWrite(C2, HIGH);
  digitalWrite(C3, HIGH);
  digitalWrite(C4, LOW);

  if (digitalRead(R1) == LOW) { read_key = 'A'; delay(300); }
  if (digitalRead(R2) == LOW) { read_key = 'B'; delay(300); }
  if (digitalRead(R3) == LOW) { read_key = 'C'; delay(300); }
  if (digitalRead(R4) == LOW) { read_key = 'D'; delay(300); }

  return read_key;
}