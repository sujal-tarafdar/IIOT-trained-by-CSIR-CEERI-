// Constants
const int ADC_PIN = 35;   // Potentiometer pin
const int ADC_MAX = 4095;
const float VREF = 3.3;

// LEDs
const int led1 = 2;
const int led2 = 5;

void setup() {
  Serial.begin(115200);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  // Set full voltage range (0–3.3V)
  analogSetAttenuation(ADC_11db);
}

void loop() {
  // 🔹 Read analog value
  int analogValue = analogRead(ADC_PIN);

  // 🔹 Convert to voltage
  float voltage = analogValue * (VREF / ADC_MAX);

  // 🔹 Print values
  Serial.print("Analog: ");
  Serial.print(analogValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3);
  Serial.println(" V");

  // 🔹 LED Control using voltage
  if (voltage > 1.299 && voltage < 1.799) {
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
  }
  else if (voltage > 1.799 && voltage < 3.3) {
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
  }
  else {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
  }

  delay(200);
}