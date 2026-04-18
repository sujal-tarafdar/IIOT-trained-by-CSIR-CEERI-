const int buttonPin = 4;
const int ledPin = 2;

bool ledState = false;        // stores LED state
bool lastButtonState = HIGH;  // previous button state

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  int currentButtonState = digitalRead(buttonPin);

  // Detect button press (HIGH → LOW)
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    ledState = !ledState;   // toggle LED
    digitalWrite(ledPin, ledState);

    Serial.println("Button Pressed");
    delay(200); // debounce delay
  }

  lastButtonState = currentButtonState;
}