#define ENA 25
#define IN1 26
#define IN2 27

const int pwmChannel = 0;
const int pwmFreq = 5000;
const int pwmResolution = 8;  // 0–255

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(ENA, pwmChannel);

  // Motor direction
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // 50% speed
  
  ledcWrite(pwmChannel, 255);  // 128 ≈ 50% of 255
}

void loop() {
}