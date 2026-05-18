/**************************************************************************************************************************************
  EEERover Starter Example - Fixed Working Version
***************************************************************************************************************************************/
#define USE_WIFI_NINA false
#define USE_WIFI101 true
#include <WiFiWebServer.h>

const char ssid[] = "EEERover";
const char pass[] = "exhibition";
const int groupNumber = 28;

// Keep original pins since your hardware already works
const int rightDIR = 8;
const int rightEN  = 9;
const int leftDIR  = 12;
const int leftEN   = 11;


// ================= WEBPAGE =================
const char webpage[] =
"<html><head><style>\
.btn {background-color: #ddd; padding: 14px 28px; font-size: 16px; margin: 5px; border: none; cursor: pointer;}\
.btn:hover {background: #eee;}\
</style></head>\
<body>\
<button class=\"btn\" onclick=\"forward()\">Forward</button><br>\
<button class=\"btn\" onclick=\"left()\">Left</button>\
<button class=\"btn\" onclick=\"stopRover()\">Stop</button>\
<button class=\"btn\" onclick=\"right()\">Right</button><br>\
<button class=\"btn\" onclick=\"reverse()\">Reverse</button>\
<br>DIRECTION: <span id=\"dir\">STOPPED</span>\
<script>\
var xhttp = new XMLHttpRequest();\
xhttp.onreadystatechange = function() {\
  if (this.readyState == 4 && this.status == 200) {\
    document.getElementById(\"dir\").innerHTML = this.responseText;\
  }\
};\
function forward() {xhttp.open(\"GET\", \"/forward\"); xhttp.send();}\
function reverse() {xhttp.open(\"GET\", \"/reverse\"); xhttp.send();}\
function right() {xhttp.open(\"GET\", \"/right\"); xhttp.send();}\
function left() {xhttp.open(\"GET\", \"/left\"); xhttp.send();}\
function stopRover() {xhttp.open(\"GET\", \"/stop\"); xhttp.send();}\
</script></body></html>";

WiFiWebServer server(80);


// ================= WEB HANDLERS =================
void handleRoot() {
  server.send(200, F("text/html"), webpage);
}


// ================= MOVEMENT FUNCTIONS =================
void moveForward() {
  Serial.println("FORWARD");

  digitalWrite(leftDIR, LOW);
  digitalWrite(leftEN, HIGH);

  digitalWrite(rightDIR, LOW);
  digitalWrite(rightEN, HIGH);

  server.send(200, F("text/plain"), F("FORWARD"));
}


void moveReverse() {
  digitalWrite(leftDIR,  HIGH);  
  digitalWrite(leftEN, HIGH);   
  digitalWrite(rightDIR,  HIGH);
  digitalWrite(rightEN, HIGH);
  server.send(200, F("text/plain"), F("REVERSE"));
}

// FIXED RIGHT TURN
void moveRight() {
  Serial.println("RIGHT");

  digitalWrite(leftDIR, LOW);
  digitalWrite(leftEN, HIGH);  //left wheel forward

  digitalWrite(rightDIR, LOW);   
  digitalWrite(rightEN, LOW);  

  server.send(200, F("text/plain"), F("RIGHT"));
}


// FIXED LEFT TURN
void moveLeft() {
  Serial.println("LEFT");

  digitalWrite(leftDIR, LOW);
  digitalWrite(leftEN, LOW); 

  digitalWrite(rightDIR, LOW);  
  digitalWrite(rightEN, HIGH);  //rightwheel forward

  server.send(200, F("text/plain"), F("LEFT"));
}


// ADDED STOP FUNCTION
void stopMotors() {
  Serial.println("STOP");

  digitalWrite(leftEN, LOW);
  digitalWrite(rightEN, LOW);

  server.send(200, F("text/plain"), F("STOPPED"));
}


void handleNotFound() {
  server.send(404, F("text/plain"), F("Not Found"));
}


// ================= SETUP =================
void setup() {
  pinMode(rightDIR, OUTPUT);
  pinMode(rightEN, OUTPUT);
  pinMode(leftDIR, OUTPUT);
  pinMode(leftEN, OUTPUT);

  // Start with motors off
  digitalWrite(leftEN, LOW);
  digitalWrite(rightEN, LOW);

  Serial.begin(9600);
  while (!Serial && millis() < 1000);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("No WiFi shield detected");
    while (true);
  }

  if (groupNumber) {
    WiFi.config(IPAddress(192, 168, 0, groupNumber + 1));
  }

  Serial.println("Connecting to WiFi...");

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    Serial.println("Retrying WiFi...");
    delay(500);
  }

  Serial.println("WiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());


  // Routes
  server.on(F("/"), handleRoot);
  server.on(F("/forward"), moveForward);
  server.on(F("/reverse"), moveReverse);
  server.on(F("/right"), moveRight);
  server.on(F("/left"), moveLeft);
  server.on(F("/stop"), stopMotors);

  server.onNotFound(handleNotFound);

  server.begin();

  Serial.println("Web server started");
}


// ================= LOOP =================
void loop() {
  server.handleClient();
}
