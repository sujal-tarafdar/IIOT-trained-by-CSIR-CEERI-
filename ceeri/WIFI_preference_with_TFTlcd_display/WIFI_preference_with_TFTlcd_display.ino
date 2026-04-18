#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// ---------------- TOUCH ----------------
#define TOUCH_CS  33
#define TOUCH_IRQ 34
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// ---------------- TFT ----------------
TFT_eSPI tft = TFT_eSPI();

// ---------------- WEB ----------------
WebServer server(80);

// ---------------- STATES ----------------
enum ScreenState { HOME, WIFI_LIST, PASSWORD, CONNECTING, ATTENDANCE };
ScreenState screen = HOME;

// ---------------- VARIABLES ----------------
int x, y;
String ssidList[15];
int wifiCount = 0;
String selectedSSID = "";
String password = "";

// -------- GOOGLE SCRIPT URL --------
String scriptURL = "https://script.google.com/macros/s/AKfycbzY0KPG02ZvtXRB7psPMBL3TKKhyQqmDEMITr9Vn2Mff5H9pASecfhI_Iq5wrnBIgpsFA/exec";

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touch.begin();
  touch.setRotation(1);

  drawHome();
}

// ================= LOOP =================
void loop() {

  if (touch.touched()) {
    TS_Point p = touch.getPoint();

    x = map(p.x, 3793, 503, 0, 320);
    y = map(p.y, 3665, 474, 0, 240);

    handleTouch(x, y);
    delay(150);
  }

  server.handleClient();
}

// ================= TOUCH =================
void handleTouch(int x, int y) {

  // BACK BUTTON
  if (x < 50 && y < 40 && screen != HOME) {
    drawHome();
    return;
  }

  // HOME
  if (screen == HOME) {
    if (y > 80 && y < 140) drawWiFi();
    if (y > 150 && y < 210) drawAttendance();
  }

  // WIFI LIST
  else if (screen == WIFI_LIST) {

    if (x > 260 && y < 50) {
      drawWiFi();
      return;
    }

    if (x > 200 && x < 260 && y < 50) {
      WiFi.disconnect(true);
      drawWiFi();
      return;
    }

    for (int i = 0; i < wifiCount; i++) {
      int yPos = 60 + i * 35;
      if (y > yPos && y < yPos + 30) {
        selectedSSID = ssidList[i];
        drawPassword();
      }
    }
  }

  // PASSWORD
  else if (screen == PASSWORD) {

    for (int r = 0; r < 3; r++) {
      for (int c = 0; c < 3; c++) {
        int bx = 40 + c * 80;
        int by = 100 + r * 40;

        if (x > bx && x < bx + 60 && y > by && y < by + 30) {
          password += String(r * 3 + c + 1);
          updatePassword();
        }
      }
    }

    if (x > 120 && x < 180 && y > 220) {
      password += "0";
      updatePassword();
    }

    if (x > 40 && x < 100 && y > 220) {
      if (password.length()) password.remove(password.length() - 1);
      updatePassword();
    }

    if (x > 200 && y > 220) {
      connectWiFi(selectedSSID, password);
    }
  }

  // ATTENDANCE SCREEN
  else if (screen == ATTENDANCE) {

    // MARK PRESENT BUTTON
    if (x > 60 && x < 260 && y > 100 && y < 160) {
      sendAttendance("101", "Ayush", "Present");
    }

    // MARK ABSENT BUTTON
    if (x > 60 && x < 260 && y > 170 && y < 230) {
      sendAttendance("101", "Ayush", "Absent");
    }
  }
}

// ================= HEADER =================
void drawHeader(String title) {
  tft.fillRect(0, 0, 320, 50, TFT_BLUE);

  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(title, 160, 25);

  if (screen != HOME) {
    tft.setTextDatum(ML_DATUM);
    tft.drawString("<", 10, 25);
  }
}

// ================= HOME =================
void drawHome() {

  screen = HOME;
  tft.fillScreen(TFT_BLACK);

  drawHeader("SMART PANEL");

  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);

  if (WiFi.status() == WL_CONNECTED)
    tft.drawString("WiFi: ON", 10, 10);
  else
    tft.drawString("WiFi: OFF", 10, 10);

  drawCard(80, "WiFi Manager");
  drawCard(150, "Attendance");
}

// ================= CARD =================
void drawCard(int y, String text) {

  tft.fillRoundRect(40, y, 240, 50, 10, TFT_DARKGREY);

  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(text, 160, y + 25);
}

// ================= WIFI =================
void drawWiFi() {

  screen = WIFI_LIST;
  tft.fillScreen(TFT_BLACK);

  drawHeader("WiFi Networks");

  tft.fillCircle(290, 25, 12, TFT_GREEN);
  tft.drawString("R", 290, 25);

  tft.fillCircle(240, 25, 12, TFT_RED);
  tft.drawString("X", 240, 25);

  wifiCount = WiFi.scanNetworks();

  for (int i = 0; i < wifiCount && i < 10; i++) {

    int yPos = 60 + i * 35;
    ssidList[i] = WiFi.SSID(i);

    tft.fillRoundRect(10, yPos, 300, 30, 6, TFT_DARKGREY);

    tft.setTextDatum(ML_DATUM);
    tft.drawString(ssidList[i], 20, yPos + 15);

    tft.setTextDatum(MR_DATUM);
    tft.drawString(String(WiFi.RSSI(i)), 300, yPos + 15);
  }
}

// ================= PASSWORD =================
void drawPassword() {

  screen = PASSWORD;
  password = "";

  tft.fillScreen(TFT_BLACK);
  drawHeader("Enter Password");

  tft.drawRect(40, 60, 240, 30, TFT_WHITE);
  drawKeypad();
}

// ================= KEYPAD =================
void drawKeypad() {

  int num = 1;

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {

      int x = 40 + c * 80;
      int y = 100 + r * 40;

      tft.fillRoundRect(x, y, 60, 30, 5, TFT_DARKGREY);
      tft.setTextDatum(MC_DATUM);
      tft.drawString(String(num++), x + 30, y + 15);
    }
  }

  tft.fillRoundRect(120, 220, 60, 30, 5, TFT_DARKGREY);
  tft.drawString("0", 150, 235);

  tft.fillRoundRect(40, 220, 60, 30, 5, TFT_RED);
  tft.drawString("DEL", 70, 235);

  tft.fillRoundRect(200, 220, 80, 30, 5, TFT_GREEN);
  tft.drawString("OK", 240, 235);
}

// ================= PASSWORD UPDATE =================
void updatePassword() {

  tft.fillRect(50, 65, 220, 20, TFT_BLACK);

  String stars = "";
  for (int i = 0; i < password.length(); i++) stars += "*";

  tft.setTextDatum(MC_DATUM);
  tft.drawString(stars, 160, 75);
}

// ================= WIFI CONNECT =================
void connectWiFi(String ssid, String pass) {

  screen = CONNECTING;
  tft.fillScreen(TFT_BLACK);

  drawHeader("Connecting");

  WiFi.begin(ssid.c_str(), pass.c_str());

  int t = 0;
  while (WiFi.status() != WL_CONNECTED && t < 15) {
    delay(500);
    t++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    drawHome();
  } else {
    tft.drawString("Failed", 160, 120);
  }
}

// ================= ATTENDANCE =================
void drawAttendance() {

  screen = ATTENDANCE;
  tft.fillScreen(TFT_BLACK);

  drawHeader("Attendance");

  tft.fillRoundRect(60, 100, 200, 50, 10, TFT_GREEN);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Present", 160, 125);

  tft.fillRoundRect(60, 170, 200, 50, 10, TFT_RED);
  tft.drawString("Absent", 160, 195);
}

// ================= SEND DATA =================
void sendAttendance(String id, String name, String status) {

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String url = scriptURL + "?id=" + id + "&name=" + name + "&status=" + status;

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.println("Attendance Marked");
    } else {
      Serial.println("Error sending data");
    }

    http.end();
  }
}