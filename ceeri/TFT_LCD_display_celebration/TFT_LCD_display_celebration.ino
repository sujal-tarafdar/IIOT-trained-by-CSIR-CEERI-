#include <SPI.h>
#include <TFT_eSPI.h> 
#include <XPT2046_Touchscreen.h>

// --- Touch Screen Pins ---
#define XPT2046_CS  33  // MUST MATCH YOUR WIRING
#define XPT2046_IRQ 34  // MUST MATCH YOUR WIRING

TFT_eSPI tft = TFT_eSPI(); 
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

int rawX_TL, rawY_TL; // Top-Left raw values
int rawX_BR, rawY_BR; // Bottom-Right raw values

void setup() {
  Serial.begin(115200);
  
  tft.init();
  tft.setRotation(1); // 1 = Landscape. KEEP THIS THE SAME AS YOUR MAIN CODE!
  
  ts.begin();
  ts.setRotation(1);  // KEEP THIS THE SAME AS YOUR MAIN CODE!

  calibrateTouch();
}

void loop() {
  // Nothing happens in the loop for this calibration sketch
}

void drawCrosshair(int x, int y, uint16_t color) {
  tft.drawLine(x - 10, y, x + 10, y, color);
  tft.drawLine(x, y - 10, x, y + 10, color);
}

void getTouchAverage(int &avgX, int &avgY) {
  long sumX = 0, sumY = 0;
  int count = 0;

  // Wait for touch
  while (!ts.touched()) {
    delay(10);
  }

  // Take 20 readings to average out the electrical noise
  while (ts.touched() && count < 20) {
    TS_Point p = ts.getPoint();
    sumX += p.x;
    sumY += p.y;
    count++;
    delay(5);
  }

  // Wait for release
  while (ts.touched()) {
    delay(10);
  }

  avgX = sumX / count;
  avgY = sumY / count;
}

void calibrateTouch() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  // --- STEP 1: TOP LEFT ---
  tft.drawString("Touch the crosshair", tft.width()/2, tft.height()/2, 2);
  drawCrosshair(10, 10, TFT_RED);
  
  getTouchAverage(rawX_TL, rawY_TL);
  
  tft.fillScreen(TFT_BLACK);
  delay(500);

  // --- STEP 2: BOTTOM RIGHT ---
  tft.drawString("Touch the other crosshair", tft.width()/2, tft.height()/2, 2);
  drawCrosshair(tft.width() - 10, tft.height() - 10, TFT_GREEN);
  
  getTouchAverage(rawX_BR, rawY_BR);

  // --- STEP 3: DISPLAY RESULTS ---
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Done! Check Serial Monitor.", tft.width()/2, tft.height()/2, 2);

  Serial.println("\n\n====================================");
  Serial.println("🎉 CALIBRATION COMPLETE 🎉");
  Serial.println("====================================");
  Serial.println("Replace the map() functions in your main loop with these exact lines:\n");
  
  Serial.printf("int touch_x = map(p.x, %d, %d, 10, %d);\n", rawX_TL, rawX_BR, tft.width() - 10);
  Serial.printf("int touch_y = map(p.y, %d, %d, 10, %d);\n", rawY_TL, rawY_BR, tft.height() - 10);
  
  Serial.println("\n(Note: If your X and Y axes are swapped when you touch the screen, ");
  Serial.println(" simply swap p.x and p.y in those lines!)");
  Serial.println("====================================");
}