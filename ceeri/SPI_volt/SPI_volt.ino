#include <ADS1120.h>

#define CS    5
#define DRDY  4

ADS1120 ads1120;

// ✅ Correct reference
float VREF = 3.3;

void setup() {
  Serial.begin(115200);

  ads1120.begin(CS, DRDY);

  ads1120.setGain(1);
  ads1120.setDataRate(0x06);
  ads1120.setOpMode(0x00);
  ads1120.setConversionMode(0x01);
  ads1120.setVoltageRef(0);   // internal 2.048V
  ads1120.setTemperatureMode(0);

  ads1120.setMultiplexer(0x0A); // AIN2 - AIN3

  Serial.println("ADC Value\tVoltage (V)");
}

void loop() {

  int16_t adcValue = ads1120.readADC_Single();

  float voltage = (adcValue / 32768.0) * VREF;

  Serial.print("AV:");
  Serial.print(adcValue);
  Serial.print("\tDV:");
  Serial.println(voltage, 6);

  delay(300);
}