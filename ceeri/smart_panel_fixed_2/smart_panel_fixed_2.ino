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
int wifiCount = 0;
String selectedSSID = "";
String password = "";

String rollNumber = "";
String studentName = "";

String scriptURL = "YOUR_SCRIPT_URL";

// ================= PROTOTYPES =================
void handleTouch(int x, int y);
void drawHome();
void drawWiFi();
void drawPassword();
void drawRollInput();
void drawStudent();
void updatePassword();
void updateRoll();
void getStudentData();
void sendAttendance(String id, String name, String status);
void connectWiFi(String ssid, String pass);
// FIX #3: Added URL encoding helper
String urlEncode(String str);

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
}

// ================= TOUCH =================
void handleTouch(int x, int y) {

  if (x < 50 && y < 40 && screen != HOME) {
    drawHome();
    return;
  }

  if (screen == HOME) {
    if (y > 70 && y < 130) drawWiFi();
    if (y > 140 && y < 200) drawRollInput();
  }

  else if (screen == WIFI_LIST) {

    // Refresh
    if (x > 260 && y < 50) {
      drawWiFi();
      return;
    }

    // Disconnect
    if (x > 200 && x < 260 && y < 50) {
      WiFi.disconnect(true);
      drawWiFi();
      return;
    }

    for (int i = 0; i < wifiCount && i < 8; i++) {
      int yPos = 60 + i * 35;
      if (y > yPos && y < yPos + 30) {
        selectedSSID = ssidList[i];
        drawPassword();
      }
    }
  }

  else if (screen == PASSWORD) {

    char keys[4][3] = {
      {'1','2','3'},
      {'4','5','6'},
      {'7','8','9'},
      {'*','0','#'}
    };

    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {

        int bx = 60 + c * 70;
        int by = 100 + r * 40;

        if (x > bx && x < bx + 60 && y > by && y < by + 30) {

          char key = keys[r][c];

          if (key >= '0' && key <= '9') password += key;
          else if (key == '*') {
            if (password.length()) password.remove(password.length() - 1);
          }
          else if (key == '#') {
            connectWiFi(selectedSSID, password);
            // FIX #7: Show connecting status instead of silently going home
            tft.fillScreen(TFT_BLACK);
            drawHeader("Connecting...");
            tft.setTextDatum(MC_DATUM);
            tft.setTextColor(TFT_WHITE);
            tft.drawString(selectedSSID, 160, 120);

            // Wait up to 10 seconds for connection
            int tries = 0;
            while (WiFi.status() != WL_CONNECTED && tries < 20) {
              delay(500);
              tries++;
            }

            if (WiFi.status() == WL_CONNECTED) {
              tft.fillScreen(TFT_BLACK);
              drawHeader("Connected!");
              tft.drawString(WiFi.localIP().toString(), 160, 120);
              delay(1500);
            } else {
              tft.fillScreen(TFT_BLACK);
              drawHeader("Failed!");
              tft.drawString("Check password", 160, 120);
              delay(1500);
            }

            drawHome();
          }

          updatePassword();
        }
      }
    }
  }

  else if (screen == ROLL_INPUT) {

    // FIX #4: Corrected keymap — each key is unique and useful
    // Row 4 was {'0','0','0','E'} which had no Backspace, Clear, or Space
    char keys[4][4] = {
      {'1','2','3','B'},  // B = Backspace
      {'4','5','6','C'},  // C = Clear all
      {'7','8','9','E'},  // E = Enter/Submit  <-- moved here from row 4
      {'*','0','#','S'}   // S = Space (if needed), * and # unused/reserved
    };

    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {

        int bx = 30 + c * 70;
        int by = 90 + r * 40;

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
          // FIX #1: Removed the 'S' (space) key from roll number input.
          // Spaces in a roll number break the HTTP GET URL and make no sense as an ID.
          // If you truly need alphanumeric IDs, implement a full alpha keypad instead.
          else if (key == 'E') {
            if (rollNumber.length() > 0) {  // Don't submit empty roll number
              getStudentData();
              drawStudent();
            }
          }
        }
      }
    }
  }

  else if (screen == STUDENT) {

    if (x > 60 && x < 260 && y > 120 && y < 170)
      sendAttendance(rollNumber, studentName, "Present");

    if (x > 60 && x < 260 && y > 180 && y < 230)
      sendAttendance(rollNumber, studentName, "Absent");
  }
}

// ================= UI =================
void drawHeader(String title) {
  tft.fillRect(0, 0, 320, 50, TFT_BLUE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(title, 160, 25);
}

void drawHome() {
  screen = HOME;
  tft.fillScreen(TFT_BLACK);
  drawHeader("SMART PANEL");

  tft.fillRoundRect(40, 80, 240, 50, 10, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("WiFi Manager", 160, 105);

  tft.fillRoundRect(40, 150, 240, 50, 10, TFT_DARKGREY);
  tft.drawString("Attendance", 160, 175);
}

// ================= WIFI =================
void drawWiFi() {
  screen = WIFI_LIST;
  tft.fillScreen(TFT_BLACK);
  drawHeader("WiFi");

  tft.fillCircle(290, 25, 12, TFT_GREEN);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("R", 290, 25);

  tft.fillCircle(230, 25, 12, TFT_RED);
  tft.drawString("X", 230, 25);

  // FIX #7: Show scanning message while scanning
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("Scanning...", 160, 120);

  wifiCount = WiFi.scanNetworks();

  tft.fillRect(0, 50, 320, 190, TFT_BLACK);  // Clear scanning message

  if (wifiCount == 0) {
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("No networks found", 160, 120);
    return;
  }

  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(ML_DATUM);  // Left-align SSIDs

  for (int i = 0; i < wifiCount && i < 8; i++) {
    int yPos = 60 + i * 35;
    ssidList[i] = WiFi.SSID(i);

    // Show lock icon for secured networks
    uint16_t color = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? TFT_DARKGREY : TFT_NAVY;
    tft.fillRoundRect(10, yPos, 300, 30, 8, color);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(ssidList[i], 20, yPos + 15);
  }
}

// ================= PASSWORD =================
void drawPassword() {
  screen = PASSWORD;
  password = "";

  tft.fillScreen(TFT_BLACK);
  drawHeader("Password");

  // Password input box
  tft.drawRect(40, 60, 240, 30, TFT_WHITE);

  // FIX #2 (partial): Draw the numeric keypad visually
  // NOTE: This keypad only handles numeric passwords.
  // If your WiFi uses letters, you need a full QWERTY implementation.
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);

  char keys[4][3] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
  };
  const char* labels[4][3] = {
    {"1","2","3"},
    {"4","5","6"},
    {"7","8","9"},
    {"<","0","OK"}
  };

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 3; c++) {
      int bx = 60 + c * 70;
      int by = 100 + r * 40;
      tft.fillRoundRect(bx, by, 60, 30, 6, TFT_DARKGREY);
      tft.setTextColor(TFT_WHITE);
      tft.drawString(labels[r][c], bx + 30, by + 15);
    }
  }
}

// ================= ROLL =================
void drawRollInput() {
  screen = ROLL_INPUT;
  rollNumber = "";

  tft.fillScreen(TFT_BLACK);
  drawHeader("Enter Roll No.");

  // FIX #5: Draw the roll number display box and keypad visually
  tft.drawRect(40, 60, 240, 25, TFT_WHITE);  // Input display box

  // FIX #4: Updated labels to match corrected keymap
  const char* labels[4][4] = {
    {"1","2","3","<"},     // < = Backspace
    {"4","5","6","CLR"},   // CLR = Clear
    {"7","8","9","OK"},    // OK = Submit
    {" ","0"," "," "}      // only 0 is useful here; blanks are inactive
  };

  tft.setTextDatum(MC_DATUM);

  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 4; c++) {
      int bx = 30 + c * 70;
      int by = 90 + r * 40;

      // Don't draw blank/inactive buttons
      String lbl = String(labels[r][c]);
      lbl.trim();
      if (r == 3 && c != 1) continue;  // Only draw '0' in last row

      uint16_t btnColor = (c == 3) ? TFT_NAVY : TFT_DARKGREY;  // Highlight action column
      tft.fillRoundRect(bx, by, 60, 30, 6, btnColor);
      tft.setTextColor(TFT_WHITE);
      tft.drawString(labels[r][c], bx + 30, by + 15);
    }
  }
}

// ================= STUDENT =================
void getStudentData() {
  // Student database — roll number : name
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
  drawHeader("Student");

  // FIX #6: Draw the Present and Absent buttons visually
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);

  // Student info
  tft.setTextColor(TFT_CYAN);
  tft.drawString("Roll: " + rollNumber, 160, 75);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(studentName, 160, 100);

  // Present button (green)
  tft.fillRoundRect(60, 120, 200, 50, 10, TFT_DARKGREEN);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("PRESENT", 160, 145);

  // Absent button (red)
  tft.fillRoundRect(60, 180, 200, 50, 10, TFT_RED);
  tft.drawString("ABSENT", 160, 205);
}

// ================= UPDATE =================
void updatePassword() {
  tft.fillRect(50, 65, 220, 20, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  // Show asterisks for security
  String masked = "";
  for (int i = 0; i < password.length(); i++) masked += "*";
  tft.drawString(masked, 160, 75);
}

void updateRoll() {
  tft.fillRect(50, 65, 220, 16, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(rollNumber, 160, 73);
}

// ================= WIFI =================
void connectWiFi(String ssid, String pass) {
  WiFi.begin(ssid.c_str(), pass.c_str());
  // Connection result is checked in handleTouch after calling this
}

// ================= FIX #3: URL encoding =================
String urlEncode(String str) {
  String encoded = "";
  char c;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (isAlphaNumeric(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded += c;
    } else if (c == ' ') {
      encoded += '+';
    } else {
      encoded += '%';
      if (c < 16) encoded += '0';
      encoded += String(c, HEX);
    }
  }
  return encoded;
}

// ================= SEND =================
void sendAttendance(String id, String name, String status) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // FIX #3: URL-encode all parameters to prevent broken requests
    String url = scriptURL
      + "?id="     + urlEncode(id)
      + "&name="   + urlEncode(name)
      + "&status=" + urlEncode(status);

    http.begin(url);
    int httpCode = http.GET();
    http.end();

    // FIX #7: Show result feedback to user
    tft.fillRect(60, 115, 200, 125, TFT_BLACK);  // Clear button area
    tft.setTextDatum(MC_DATUM);

    if (httpCode == 200) {
      tft.setTextColor(TFT_GREEN);
      tft.drawString("Sent: " + status, 160, 160);
    } else {
      tft.setTextColor(TFT_RED);
      tft.drawString("Error: " + String(httpCode), 160, 160);
    }

    delay(1500);
    drawHome();

  } else {
    // FIX #7: Notify user when WiFi is not connected
    tft.fillRect(60, 115, 200, 125, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("No WiFi!", 160, 160);
    delay(1500);
    drawHome();
  }
}
