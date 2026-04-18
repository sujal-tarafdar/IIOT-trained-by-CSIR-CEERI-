#include <WiFi.h>
#include <WebServer.h>

// ================= WIFI =================
const char* ssid = "SUJAL1";
const char* password = "SUJAL1234";

// ================= MOTOR PINS =================
#define ENA 25
#define IN1 26
#define IN2 27

WebServer server(80);

// ================= PWM =================
const int pwmChannel = 0;
const int pwmFreq = 5000;
const int pwmResolution = 8;

int speedValue = 128; // default 50%

// ================= WEBPAGE =================
String webpage = R"rawliteral(
<!DOCTYPE html>
<html class="dark" lang="en">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
<title>KINETIC_OS | CONTROL_UNIT_01</title>

<script src="https://cdn.tailwindcss.com"></script>

<style>
body {
  background:#0d0f0f;
  color:#e1e7e6;
  font-family: Arial;
  text-align:center;
}

h1 {
  color:#00e475;
}

.slider {
  width: 80%;
}

button {
  padding:15px 25px;
  margin:10px;
  font-size:16px;
  border:none;
  border-radius:8px;
  cursor:pointer;
}

.forward { background:#00e475; }
.stop { background:red; color:white; }
.reverse { background:#f1b600; }
</style>
</head>

<body>

<h1>🚀 KINETIC_OS CONTROL</h1>

<h2>Speed: <span id="speedDisplay">50</span>%</h2>

<input type="range" min="0" max="100" value="50" class="slider"
oninput="updateSpeed(this.value)">

<br><br>

<button class="forward" onclick="fetch('/forward')">⬆ Forward</button>
<button class="stop" onclick="fetch('/stop')">⛔ Stop</button>
<button class="reverse" onclick="fetch('/reverse')">⬇ Reverse</button>

<script>
function updateSpeed(val){
  document.getElementById("speedDisplay").innerText = val;
  fetch("/speed?value=" + val);
}
</script>

</body>
</html>
)rawliteral";

// ================= ROUTES =================
void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleForward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  server.send(200, "text/plain", "Forward");
}

void handleReverse() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  server.send(200, "text/plain", "Reverse");
}

void handleStop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  server.send(200, "text/plain", "Stopped");
}

void handleSpeed() {
  if (server.hasArg("value")) {
    int val = server.arg("value").toInt();
    speedValue = map(val, 0, 100, 0, 255);
    ledcWrite(pwmChannel, speedValue);
  }
  server.send(200, "text/plain", "Speed Updated");
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  ledcSetup(pwmChannel, pwmFreq, pwmResolution);
  ledcAttachPin(ENA, pwmChannel);

  ledcWrite(pwmChannel, speedValue);

  // WiFi connect
  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Routes
  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/reverse", handleReverse);
  server.on("/stop", handleStop);
  server.on("/speed", handleSpeed);

  server.begin();
}

// ================= LOOP =================
void loop() {
  server.handleClient();
}