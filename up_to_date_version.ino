/**************************************************************************************************************************************
  EEELunarRover - Master Sketch - Group 28
***************************************************************************************************************************************/
#define USE_WIFI_NINA false
#define USE_WIFI101 true
#include <WiFiWebServer.h>

const char ssid[] = "EEERover";
const char pass[] = "exhibition";
const int groupNumber = 28;
String uartBuffer = "";

// Motor pins
const int rightDIR = 8;
const int rightEN  = 9;
const int leftDIR  = 12;
const int leftEN   = 11;

// Sensor pins
const int IR_PIN  = 3;
const int US_PIN  = 6;
const int MAG_PIN = A3;

// IR pulse counting
volatile int irPulseCount  = 0;
unsigned long irLastSample = 0;
const int IR_SAMPLE_WINDOW = 500;

// Sensor data
String rockAge            = "----";
String irRate             = "---";
String ultrasonicDetected = "unknown";
String magneticDir        = "unknown";
String rockType           = "unknown";

// TEST MODE — set false when real sensors are connected
// While true: type b/g/r/l/x in Serial Monitor to simulate rock types
bool TEST_MODE = false;

WiFiWebServer server(80);



// ================= WEB HANDLERS =================
void addCors() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
}

void handleRoot() {
  addCors();
  server.send(200, F("text/plain"), F("Rover API is running"));
}

void handleNotFound() {
  addCors();
  server.send(404, F("text/plain"), F("Not Found"));
}

void handleSensors() {
  addCors();
  String json = "{";
  json += "\"age\":\""        + rockAge            + "\",";
  json += "\"ir\":\""         + irRate             + "\",";
  json += "\"ultrasonic\":\"" + ultrasonicDetected + "\",";
  json += "\"magnetic\":\""   + magneticDir        + "\",";
  json += "\"type\":\""       + rockType           + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

// ================= MOVEMENT FUNCTIONS =================

// Speed settings
const int FAST_SPEED = 255;   // full speed
const int SLOW_SPEED = 128;   // slower wheel for diagonal movement
const int STOP_SPEED = 0;

void moveForward() {
  Serial.println("FORWARD");

  digitalWrite(leftDIR, LOW);
  analogWrite(leftEN, FAST_SPEED);

  digitalWrite(rightDIR, LOW);
  analogWrite(rightEN, FAST_SPEED);

  addCors();
  server.send(200, F("text/plain"), F("FORWARD"));
}

void moveReverse() {
  Serial.println("REVERSE");

  digitalWrite(leftDIR, HIGH);
  analogWrite(leftEN, FAST_SPEED);

  digitalWrite(rightDIR, HIGH);
  analogWrite(rightEN, FAST_SPEED);

  addCors();
  server.send(200, F("text/plain"), F("REVERSE"));
}

void moveRight() {
  Serial.println("RIGHT");

  // Pivot right: left wheel forward, right wheel reverse
  digitalWrite(leftDIR, LOW);
  analogWrite(leftEN, FAST_SPEED);

  digitalWrite(rightDIR, HIGH);
  analogWrite(rightEN, FAST_SPEED);

  addCors();
  server.send(200, F("text/plain"), F("RIGHT"));
}

void moveLeft() {
  Serial.println("LEFT");

  // Pivot left: left wheel reverse, right wheel forward
  digitalWrite(leftDIR, HIGH);
  analogWrite(leftEN, FAST_SPEED);

  digitalWrite(rightDIR, LOW);
  analogWrite(rightEN, FAST_SPEED);

  addCors();
  server.send(200, F("text/plain"), F("LEFT"));
}

void moveForwardLeft() {
  Serial.println("FORWARD_LEFT");

  // Smooth forward-left:
  // left wheel slower, right wheel faster
  digitalWrite(leftDIR, LOW);
  analogWrite(leftEN, SLOW_SPEED);

  digitalWrite(rightDIR, LOW);
  analogWrite(rightEN, FAST_SPEED);

  addCors();
  server.send(200, F("text/plain"), F("FORWARD_LEFT"));
}

void moveForwardRight() {
  Serial.println("FORWARD_RIGHT");

  // Smooth forward-right:
  // left wheel faster, right wheel slower
  digitalWrite(leftDIR, LOW);
  analogWrite(leftEN, FAST_SPEED);

  digitalWrite(rightDIR, LOW);
  analogWrite(rightEN, SLOW_SPEED);

  addCors();
  server.send(200, F("text/plain"), F("FORWARD_RIGHT"));
}

void stopMotors() {
  Serial.println("STOP");

  analogWrite(leftEN, STOP_SPEED);
  analogWrite(rightEN, STOP_SPEED);

  addCors();
  server.send(200, F("text/plain"), F("STOPPED"));
}

// ================= ROCK CLASSIFIER =================
void classifyRock() {
  if (irRate == "---" || ultrasonicDetected == "unknown" || magneticDir == "unknown") {
    rockType = "unknown";
    return;
  }
  bool highIR  = (irRate.toInt() > 400);
  bool hasUS   = (ultrasonicDetected == "yes");
  bool fieldUp = (magneticDir == "up");

  if      ( highIR &&  hasUS  ) rockType = "Basaltoid";
  else if (!highIR && !hasUS ) rockType = "Gravion";
  else if (!highIR &&  hasUS ) rockType = "Regolix";
  else if ( highIR && !hasUS ) rockType = "Lunarite";
  else                                    rockType = "unknown";

  Serial.println("Rock type: " + rockType);
}

// ================= IR INTERRUPT =================
void irPulseISR() {
  irPulseCount++;
}

// ================= SENSOR UPDATE FUNCTIONS =================
void updateIR() {
  unsigned long now = millis();
  if (now - irLastSample >= IR_SAMPLE_WINDOW) {
    int count    = irPulseCount;
    irPulseCount = 0;
    irLastSample = now;
    irRate       = String(count * (1000 / IR_SAMPLE_WINDOW));
    Serial.println("IR rate: " + irRate + " /s");
    classifyRock();
  }
}

void updateUltrasonic() {
  String prev = ultrasonicDetected;
  ultrasonicDetected = digitalRead(US_PIN) ? "yes" : "no";
  if (ultrasonicDetected != prev) {
    Serial.println("Ultrasonic: " + ultrasonicDetected);
    classifyRock();
  }
}

void updateMagnetic() {
  int reading = analogRead(MAG_PIN);
  String prev = magneticDir;
  magneticDir = (reading > 800) ? "up" : "down";
  //if (magneticDir != prev) {
    Serial.println("Magnetic: " + magneticDir + " (raw: " + reading + ")");
    classifyRock();
  //}
}

// ================= TEST FUNCTION =================
// Type in Serial Monitor: b=Basaltoid  g=Gravion  r=Regolix  l=Lunarite  x=clear
void runTestMode() {
  if (!Serial.available()) return;
  char cmd = Serial.read();

  if      (cmd == 'b') { irRate="547"; ultrasonicDetected="yes"; magneticDir="down"; rockAge="#200"; }
  else if (cmd == 'g') { irRate="312"; ultrasonicDetected="no";  magneticDir="down"; rockAge="#450"; }
  else if (cmd == 'r') { irRate="312"; ultrasonicDetected="yes"; magneticDir="up";   rockAge="#099"; }
  else if (cmd == 'l') { irRate="547"; ultrasonicDetected="no";  magneticDir="up";   rockAge="#750"; }
  else if (cmd == 'x') { irRate="---"; ultrasonicDetected="unknown"; magneticDir="unknown"; rockAge="----"; rockType="unknown"; return; }
  else if (cmd == 'a') {
    // Simulate receiving "#123" over UART
    uartBuffer = "";
    String fake = "#123";
    for (int i = 0; i < fake.length(); i++) {
      char c = fake[i];
      if (c == '#') {
        uartBuffer = "#";
      } else if (uartBuffer.startsWith("#")) {
        uartBuffer += c;
        if (uartBuffer.length() == 4) {
          if (isDigit(uartBuffer[1]) && isDigit(uartBuffer[2]) && isDigit(uartBuffer[3])) {
            rockAge = uartBuffer;
            Serial.println("Rock age decoded: " + rockAge);
          }
          uartBuffer = "";
        }
      }
    }
    return;
  }
  else { Serial.println("Commands: b=Basaltoid  g=Gravion  r=Regolix  l=Lunarite  x=clear  a=fakeRadio"); return; }

  classifyRock();
}


// ================= SETUP =================
void setup() {
  pinMode(rightDIR, OUTPUT);
  pinMode(rightEN,  OUTPUT);
  pinMode(leftDIR,  OUTPUT);
  pinMode(leftEN,   OUTPUT);
  digitalWrite(leftEN,  LOW);
  digitalWrite(rightEN, LOW);

  pinMode(IR_PIN,  INPUT);
  pinMode(US_PIN,  INPUT);
  pinMode(MAG_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(IR_PIN), irPulseISR, RISING);

  Serial.begin(9600);
  Serial1.begin(600);
  while (!Serial && millis() < 1000);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("No WiFi shield detected");
    while (true);
  }
  if (groupNumber) WiFi.config(IPAddress(192, 168, 0, groupNumber + 1));

  Serial.println("Connecting to WiFi...");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.println("Retrying...");
    delay(500);
  }
  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
 IPAddress ip = WiFi.localIP();
 Serial.print(ip[0]); Serial.print(".");
 Serial.print(ip[1]); Serial.print(".");
 Serial.print(ip[2]); Serial.print(".");
 Serial.println(ip[3]);

  server.on(F("/"),        handleRoot);
  server.on(F("/forward"), moveForward);
  server.on(F("/reverse"), moveReverse);
  server.on(F("/right"),   moveRight);
  server.on(F("/left"),    moveLeft);
  server.on(F("/stop"),    stopMotors);
  server.on(F("/forwardleft"), moveForwardLeft);
  server.on(F("/forwardright"), moveForwardRight);
  server.on(F("/sensors"), handleSensors);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Web server started");
  if (TEST_MODE) Serial.println("TEST MODE ON — type b/g/r/l/x in Serial Monitor");
}
void readRadioSignal() {
  while (Serial1.available()) {
    char c = Serial1.read();
   
    Serial.print("UART received: ");
    Serial.println(c);

    if (c == '#') {
      uartBuffer = "#";
      Serial.println("New packet started");

    } else if (uartBuffer.startsWith("#")) {
      uartBuffer += c;

      if (uartBuffer.length() == 4) {
        if (isDigit(uartBuffer[1]) &&
            isDigit(uartBuffer[2]) &&
            isDigit(uartBuffer[3])) {
          rockAge = uartBuffer;
          Serial.println("Rock age decoded: " + rockAge);
        } else {
          Serial.println("Corrupt packet ignored: " + uartBuffer);
        }
        uartBuffer = "";
      }
    }
  }
}
// ================= LOOP =================
void loop() {
  server.handleClient();
  readRadioSignal();

  if (TEST_MODE) {
    runTestMode();
  } else {
    updateIR();
    updateUltrasonic();
    updateMagnetic();
    delay(500);
  }
}
