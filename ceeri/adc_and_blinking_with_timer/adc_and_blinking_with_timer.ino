#define LED 18
#define ADC_PIN 34   // ESP32 ADC pin

hw_timer_t *timerLED = NULL;
hw_timer_t *timerADC = NULL;

volatile bool readADC = false;

// 🔴 Timer for LED
void IRAM_ATTR onLEDTimer() {
  digitalWrite(LED, !digitalRead(LED)); // toggle LED
}

// 🔵 Timer for ADC
void IRAM_ATTR onADCTimer() {
  readADC = true; // trigger ADC read
}

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(ADC_PIN, INPUT);

  // ===== Timer 0 → LED =====
  timerLED = timerBegin(0, 80, true);  // 1 µs tick
  timerAttachInterrupt(timerLED, &onLEDTimer, true);
  timerAlarmWrite(timerLED, 1000000, true); // 1 sec
  timerAlarmEnable(timerLED);

  // ===== Timer 1 → ADC =====
  timerADC = timerBegin(1, 80, true);  // 1 µs tick
  timerAttachInterrupt(timerADC, &onADCTimer, true);
  timerAlarmWrite(timerADC, 200000, true); // 200 ms (5 samples/sec)
  timerAlarmEnable(timerADC);
}

void loop() {
  if (readADC) {
    readADC = false;

    int adcValue = analogRead(ADC_PIN);

    // Convert to voltage (ESP32 = 12-bit ADC)
    float voltage = (adcValue / 4095.0) * 3.3;

    Serial.print("ADC Value: ");
    Serial.print(adcValue);

    Serial.print(" | Voltage: ");
    Serial.println(voltage);
  }
}