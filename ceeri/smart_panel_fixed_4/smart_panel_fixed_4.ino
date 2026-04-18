#include <TFT_eSPI.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ---------------- TOUCH ----------------
#define TOUCH_CS  33
#define TOUCH_IRQ 34
XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);

// ---------------- TFT ----------------
TFT_eSPI tft = TFT_eSPI();

// ---------------- STATES ----------------
enum ScreenState { HOME, WIFI_LIST, PASSWORD, ROLL_INPUT, STUDENT };
ScreenState screen = HOME;

// ---------------- VARIABLES ----------------
int x, y;
String ssidList[15];
int wifiCount       = 0;
String selectedSSID = "";
String password     = "";
String rollNumber   = "";
String studentName  = "";
String attendanceStatus = "Not Marked";

String scriptURL = "https://script.google.com/macros/s/AKfycbzY0KPG02ZvtXRB7psPMBL3TKKhyQqmDEMITr9Vn2Mff5H9pASecfhI_Iq5wrnBIgpsFA/exec";

// ---------------- SOFTWARE CLOCK ----------------
// Starts from 00:00:00 at boot.
// To show real time: set startEpoch = current Unix time after NTP sync.
unsigned long startEpoch = 0;
unsigned long bootMillis = 0;

String getTimeString() {
  unsigned long elapsed = (millis() - bootMillis) / 1000;
  unsigned long total   = startEpoch + elapsed;
  int h = (total / 3600) % 24;
  int m = (total / 60)   % 60;
  int s =  total         % 60;
  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", h, m, s);
  return String(buf);
}

// ================= PROTOTYPES =================
void handleTouch(int x, int y);
void drawHome();
void drawWiFi();
void drawPassword();
void drawRollInput();
void drawStudent();
void updatePassword();
void updateRoll();
void updateClock();
void drawClockWidget();
void drawBackButton();
void drawHeader(String title);
void drawHeaderWithBack(String title);
void refreshStudentStatus();
void getStudentData();
void sendAttendance(String id, String name, String status);
void connectWiFi(String ssid, String pass);
String urlEncode(String str);

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  bootMillis = millis();

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  touch.begin();
  touch.setRotation(1);

  drawHome();
}

// ================= LOOP =================
unsigned long lastClockUpdate = 0;

void loop() {
  // Tick clock every second on screens that show it
  if (millis() - lastClockUpdate >= 1000) {
    lastClockUpdate = millis();
    if (screen == HOME || screen == ROLL_INPUT) {
      updateClock();
    }
  }

  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    x = map(p.x, 3793, 503, 0, 320);
    y = map(p.y, 3665, 474, 0, 240);
    handleTouch(x, y);
    delay(150);
  }
}

// ================= CLOCK WIDGET =================
// Bottom-right corner: drawn at y=218, right-aligned at x=314
void updateClock() {
  tft.fillRect(170, 213, 145, 18, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(MR_DATUM);
  tft.setTextColor(0x4208);  // dim grey
  tft.drawString(getTimeString(), 314, 221);
}

void drawClockWidget() {
  updateClock();
}

// ================= BACK BUTTON =================
// Left-arrow triangle drawn inside header (top-left)
void drawBackButton() {
  tft.fillTriangle(8, 25, 22, 14, 22, 36, TFT_WHITE);
}

// ================= HEADER =================
void drawHeader(String title) {
  tft.fillRect(0, 0, 320, 50, TFT_NAVY);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(title, 170, 25);
}

void drawHeaderWithBack(String title) {
  drawHeader(title);
  drawBackButton();
}

// ================= TOUCH HANDLER =================
void handleTouch(int x, int y) {

  // ------ Back button (top-left, all non-HOME screens) ------
  if (x < 50 && y < 50 && screen != HOME) {
    if (screen == PASSWORD)   drawWiFi();
    else if (screen == STUDENT) drawRollInput();
    else                        drawHome();
    return;
  }

  // ------ HOME ------
  if (screen == HOME) {
    if (y > 82 && y < 142)  drawWiFi();
    if (y > 150 && y < 210) drawRollInput();
    return;
  }

  // ------ WIFI LIST ------
  if (screen == WIFI_LIST) {

    // Refresh (green circle, top-right)
    if (x > 268 && y < 46) {
      drawWiFi();
      return;
    }

    // Disconnect button: x=10..150, y=52..74
    if (x > 10 && x < 150 && y > 52 && y < 74) {
      WiFi.disconnect(true);
      tft.fillRect(0, 52, 320, 22, TFT_BLACK);
      tft.setTextSize(1);
      tft.setTextDatum(ML_DATUM);
      tft.setTextColor(TFT_YELLOW);
      tft.drawString("  Disconnected from network", 10, 63);
      delay(900);
      drawWiFi();
      return;
    }

    // Network rows start at y=78
    for (int i = 0; i < wifiCount && i < 7; i++) {
      int yPos = 78 + i * 24;
      if (y > yPos && y < yPos + 22) {
        selectedSSID = ssidList[i];
        drawPassword();
        return;
      }
    }
    return;
  }

  // ------ PASSWORD ------
  if (screen == PASSWORD) {

    char keys[4][3] = {
      {'1','2','3'},
      {'4','5','6'},
      {'7','8','9'},
      {'*','0','#'}
    };

    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        int bx = 60 + c * 70;
        int by = 105 + r * 38;

        if (x > bx && x < bx + 58 && y > by && y < by + 30) {
          char key = keys[r][c];

          if (key >= '0' && key <= '9') {
            password += key;
            updatePassword();
          }
          else if (key == '*') {
            if (password.length()) password.remove(password.length() - 1);
            updatePassword();
          }
          else if (key == '#') {
            // ---- BUG FIX: store ssid/pass locally, clear globals, wipe screen FIRST ----
            String ssidToConnect = selectedSSID;
            String passToConnect = password;
            password = "";            // clear before any screen draw
            selectedSSID = "";

            tft.fillScreen(TFT_BLACK);   // full wipe — no leftover asterisks
            drawHeader("Connecting...");
            tft.setTextSize(1);
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(ssidToConnect, 160, 120);

            connectWiFi(ssidToConnect, passToConnect);

            int tries = 0;
            while (WiFi.status() != WL_CONNECTED && tries < 20) {
              delay(500);
              tries++;
            }

            tft.fillScreen(TFT_BLACK);
            if (WiFi.status() == WL_CONNECTED) {
              drawHeader("Connected!");
              tft.setTextDatum(MC_DATUM);
              tft.setTextColor(TFT_GREEN);
              tft.drawString(WiFi.localIP().toString(), 160, 120);
            } else {
              drawHeader("Failed!");
              tft.setTextDatum(MC_DATUM);
              tft.setTextColor(TFT_RED);
              tft.drawString("Check password", 160, 120);
            }
            delay(1500);
            drawHome();   // clean home — no password artifacts
          }
          return;
        }
      }
    }
    return;
  }

  // ------ ROLL INPUT ------
  if (screen == ROLL_INPUT) {

    char keys[4][4] = {
      {'1','2','3','B'},
      {'4','5','6','C'},
      {'7','8','9','E'},
      {' ','0',' ',' '}
    };

    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
        if (r == 3 && c != 1) continue;  // only '0' active in last row

        int bx = 28 + c * 72;
        int by = 98 + r * 38;

        if (x > bx && x < bx + 60 && y > by && y < by + 30) {
          char key = keys[r][c];

          if (key >= '0' && key <= '9') {
            rollNumber += key;
            updateRoll();
          }
          else if (key == 'B') {
            if (rollNumber.length()) rollNumber.remove(rollNumber.length() - 1);
            updateRoll();
          }
          else if (key == 'C') {
            rollNumber = "";
            updateRoll();
          }
          else if (key == 'E') {
            if (rollNumber.length() > 0) {
              getStudentData();
              attendanceStatus = "Not Marked";
              drawStudent();
            }
          }
          return;
        }
      }
    }
    return;
  }

  // ------ STUDENT ------
  if (screen == STUDENT) {
    // Present button: x=15..145, y=158..194
    if (x > 15 && x < 145 && y > 158 && y < 194) {
      attendanceStatus = "Present";
      sendAttendance(rollNumber, studentName, "Present");
      return;
    }
    // Absent button: x=170..300, y=158..194
    if (x > 170 && x < 300 && y > 158 && y < 194) {
      attendanceStatus = "Absent";
      sendAttendance(rollNumber, studentName, "Absent");
      return;
    }
  }
}

// ================= HOME =================
void drawHome() {
  screen = HOME;
  tft.fillScreen(TFT_BLACK);
  drawHeader("SMART PANEL");

  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);

  // WiFi Manager button
  tft.fillRoundRect(30, 82, 260, 52, 10, TFT_NAVY);
  tft.drawRoundRect(30, 82, 260, 52, 10, TFT_CYAN);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("WiFi Manager", 160, 108);

  // Attendance button
  tft.fillRoundRect(30, 150, 260, 52, 10, 0x0340);  // dark green
  tft.drawRoundRect(30, 150, 260, 52, 10, TFT_GREEN);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("Attendance", 160, 176);

  // Mini clock
  drawClockWidget();
}

// ================= WIFI =================
void drawWiFi() {
  screen = WIFI_LIST;
  tft.fillScreen(TFT_BLACK);
  drawHeaderWithBack("WiFi Networks");

  // Refresh circle
  tft.fillCircle(295, 25, 14, 0x0340);
  tft.setTextSize(1);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("R", 295, 25);

  // Disconnect button
  tft.fillRoundRect(10, 52, 140, 22, 5, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(ML_DATUM);
  tft.drawString("  Disconnect", 10, 63);

  // Connected SSID label (right side of disconnect bar)
  if (WiFi.status() == WL_CONNECTED) {
    tft.setTextColor(TFT_GREEN);
    tft.setTextDatum(MR_DATUM);
    String connectedTo = WiFi.SSID();
    if (connectedTo.length() > 14) connectedTo = connectedTo.substring(0, 14) + "..";
    tft.drawString(connectedTo, 308, 63);
  }

  // Scanning
  tft.setTextColor(TFT_YELLOW);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Scanning...", 160, 140);

  wifiCount = WiFi.scanNetworks();

  tft.fillRect(0, 76, 320, 164, TFT_BLACK);

  if (wifiCount == 0) {
    tft.setTextColor(TFT_YELLOW);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("No networks found", 160, 140);
    return;
  }

  tft.setTextDatum(ML_DATUM);

  for (int i = 0; i < wifiCount && i < 7; i++) {
    int yPos = 78 + i * 24;
    ssidList[i] = WiFi.SSID(i);

    bool isConn = (WiFi.status() == WL_CONNECTED && WiFi.SSID() == ssidList[i]);
    uint16_t bg = isConn ? 0x0340 : TFT_DARKGREY;

    tft.fillRoundRect(6, yPos, 272, 20, 4, bg);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(ssidList[i], 14, yPos + 10);

    // Small lock indicator for secured networks
    if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) {
      tft.drawRect(283, yPos + 4, 8, 9, TFT_YELLOW);
      tft.drawRect(285, yPos + 2, 4, 5, TFT_YELLOW);
    }
  }
}

// ================= PASSWORD =================
void drawPassword() {
  screen = PASSWORD;
  password = "";

  tft.fillScreen(TFT_BLACK);
  drawHeaderWithBack("Enter Password");

  // SSID label
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(selectedSSID, 160, 60);

  // Password input box
  tft.drawRoundRect(40, 70, 240, 26, 5, TFT_WHITE);

  // Keypad
  const char* labels[4][3] = {
    {"1","2","3"},
    {"4","5","6"},
    {"7","8","9"},
    {"<","0","OK"}
  };

  tft.setTextDatum(MC_DATUM);
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 3; c++) {
      int bx = 60 + c * 70;
      int by = 105 + r * 38;
      uint16_t col = (r == 3 && c == 2) ? 0x0340 : TFT_DARKGREY;
      tft.fillRoundRect(bx, by, 58, 30, 6, col);
      tft.setTextColor(TFT_WHITE);
      tft.drawString(labels[r][c], bx + 29, by + 15);
    }
  }
}

// ================= ROLL INPUT =================
void drawRollInput() {
  screen = ROLL_INPUT;
  rollNumber = "";

  tft.fillScreen(TFT_BLACK);
  drawHeaderWithBack("Enter Roll No.");

  // Input display box
  tft.drawRoundRect(38, 57, 244, 28, 5, TFT_WHITE);

  // Keypad
  const char* labels[4][4] = {
    {"1","2","3","<"},
    {"4","5","6","CLR"},
    {"7","8","9","OK"},
    {" ","0"," "," "}
  };

  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(1);

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      if (r == 3 && c != 1) continue;

      int bx = 28 + c * 72;
      int by = 98 + r * 38;
      uint16_t col = (c == 3) ? TFT_NAVY : TFT_DARKGREY;
      tft.fillRoundRect(bx, by, 60, 30, 6, col);
      tft.setTextColor(TFT_WHITE);
      tft.drawString(labels[r][c], bx + 30, by + 15);
    }
  }

  // Mini clock
  drawClockWidget();
}

// ================= STUDENT =================
void getStudentData() {
  struct Student { const char* roll; const char* name; };

  Student db[] = {
    {"69",  "Abhijit Shobji"},
    {"101", "Aashika Singhal"},
    {"105", "Anusha Shandilya"},
    {"108", "Ayush Soni"},
    {"109", "Bhawna"},
    {"110", "Guniggahh Sethi"},
    {"119", "Lavina Garg"},
    {"120", "Pooja Bhadu"},
    {"132", "Saksham Sharma"},
    {"138", "Sujal Tarafadar"},
    {"141", "Vidit Sharma"}
  };

  int count = sizeof(db) / sizeof(db[0]);
  studentName = "Unknown";

  for (int i = 0; i < count; i++) {
    if (rollNumber == db[i].roll) {
      studentName = db[i].name;
      break;
    }
  }
}

void drawStudent() {
  screen = STUDENT;
  tft.fillScreen(TFT_BLACK);
  drawHeaderWithBack("Student Info");

  tft.setTextSize(1);

  // Info card
  tft.fillRoundRect(10, 56, 300, 92, 8, 0x1082);   // dark blue-grey fill
  tft.drawRoundRect(10, 56, 300, 92, 8, TFT_DARKGREY);

  // Row: Roll No.
  tft.setTextDatum(ML_DATUM);
  tft.setTextColor(TFT_DARKGREY);
  tft.drawString("Roll No. :", 20, 76);
  tft.setTextColor(TFT_CYAN);
  tft.drawString(rollNumber, 115, 76);

  // Row: Name
  tft.setTextColor(TFT_DARKGREY);
  tft.drawString("Name     :", 20, 100);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(studentName, 115, 100);

  // Row: Status
  tft.setTextColor(TFT_DARKGREY);
  tft.drawString("Status   :", 20, 124);
  refreshStudentStatus();

  // Present / Absent buttons
  tft.fillRoundRect(15, 158, 130, 36, 8, 0x0340);
  tft.drawRoundRect(15, 158, 130, 36, 8, TFT_GREEN);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("PRESENT", 80, 176);

  tft.fillRoundRect(170, 158, 130, 36, 8, 0x6000);
  tft.drawRoundRect(170, 158, 130, 36, 8, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("ABSENT", 235, 176);
}

// Refresh just the status value in the card (no full redraw)
void refreshStudentStatus() {
  tft.fillRect(115, 116, 190, 16, 0x1082);
  tft.setTextSize(1);
  tft.setTextDatum(ML_DATUM);
  uint16_t col = TFT_YELLOW;
  if (attendanceStatus == "Present") col = TFT_GREEN;
  else if (attendanceStatus == "Absent") col = TFT_RED;
  tft.setTextColor(col);
  tft.drawString(attendanceStatus, 115, 124);
}

// ================= UPDATE =================
void updatePassword() {
  tft.fillRoundRect(42, 72, 234, 20, 4, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  String masked = "";
  for (unsigned int i = 0; i < password.length(); i++) masked += "*";
  tft.drawString(masked, 160, 82);
}

void updateRoll() {
  tft.fillRoundRect(40, 59, 238, 22, 4, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(rollNumber, 160, 70);
}

// ================= WIFI CONNECT =================
void connectWiFi(String ssid, String pass) {
  WiFi.begin(ssid.c_str(), pass.c_str());
}

// ================= URL ENCODE =================
String urlEncode(String str) {
  String encoded = "";
  for (unsigned int i = 0; i < str.length(); i++) {
    char c = str.charAt(i);
    if (isAlphaNumeric(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else if (c == ' ') {
      encoded += '+';
    } else {
      encoded += '%';
      if ((unsigned char)c < 16) encoded += '0';
      encoded += String((unsigned char)c, HEX);
    }
  }
  return encoded;
}

// ================= SEND ATTENDANCE =================
void sendAttendance(String id, String name, String status) {

  // Show status change on card immediately
  attendanceStatus = status;
  refreshStudentStatus();

  if (WiFi.status() != WL_CONNECTED) {
    tft.fillRect(10, 200, 300, 28, TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("No WiFi! Connect first.", 160, 214);
    delay(1800);
    drawStudent();
    return;
  }

  HTTPClient http;
  String url = scriptURL
    + "?id="     + urlEncode(id)
    + "&name="   + urlEncode(name)
    + "&status=" + urlEncode(status);

  http.begin(url);
  int httpResponseCode = http.GET();
  http.end();

  tft.fillRect(10, 200, 300, 28, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextDatum(MC_DATUM);

  if (httpResponseCode > 0) {
    Serial.println("Attendance Marked: " + status);
    tft.setTextColor(TFT_GREEN);
    tft.drawString("Saved: " + status, 160, 214);
  } else {
    Serial.println("HTTP Error: " + String(httpResponseCode));
    tft.setTextColor(TFT_RED);
    tft.drawString("Send failed (" + String(httpResponseCode) + ")", 160, 214);
  }

  delay(1500);
  drawHome();
}
