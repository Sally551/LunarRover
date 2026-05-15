/**************************************************************************************************************************************
  EEERover Starter Example - Fixed
 ***************************************************************************************************************************************/
#define USE_WIFI_NINA         false
#define USE_WIFI101           true
#include <WiFiWebServer.h>

const char ssid[] = "EEERover";
const char pass[] = "exhibition";
const int groupNumber = 28;

const int rightDIR = 8;
const int rightEN = 9;
const int leftDIR = 10;
const int leftEN = 11;

// Webpage to return when root is requested
// FIX: Added missing < at the end of "left/button" and fixed the JavaScript string syntax
const char webpage[] = \
"<html><head><style>\
.btn {background-color: #ddd; padding: 14px 28px; font-size: 16px; margin: 5px; border: none; cursor: pointer;}\
.btn:hover {background: #eee;}\
</style></head>\
<body>\
<button class=\"btn\" onclick=\"ledOn()\">LED On</button>\
<button class=\"btn\" onclick=\"ledOff()\">LED Off</button><br>\
<button class=\"btn\" onclick=\"forward()\">Forward</button><br>\
<button class=\"btn\" onclick=\"left()\">Left</button>\
<button class=\"btn\" onclick=\"reverse()\">Reverse</button>\
<button class=\"btn\" onclick=\"right()\">Right</button>\
<br>LED STATE: <span id=\"state\">OFF</span>\
<br>DIRECTION: <span id=\"dir\">N/A</span>\
<script>\
var xhttp = new XMLHttpRequest();\
xhttp.onreadystatechange = function() {\
  if (this.readyState == 4 && this.status == 200) {\
    if(this.responseText == 'ON' || this.responseText == 'OFF') document.getElementById(\"state\").innerHTML = this.responseText;\
    else document.getElementById(\"dir\").innerHTML = this.responseText;\
  }\
};\
function ledOn() {xhttp.open(\"GET\", \"/on\"); xhttp.send();}\
function ledOff() {xhttp.open(\"GET\", \"/off\"); xhttp.send();}\
function forward() {xhttp.open(\"GET\", \"/forward\"); xhttp.send();}\
function reverse() {xhttp.open(\"GET\", \"/reverse\"); xhttp.send();}\
function right() {xhttp.open(\"GET\", \"/right\"); xhttp.send();}\
function left() {xhttp.open(\"GET\", \"/left\"); xhttp.send();}\
</script></body></html>";

WiFiWebServer server(80);

void handleRoot() {
  server.send(200, F("text/html"), webpage);
}

void ledON() {
  digitalWrite(LED_BUILTIN, 1);
  server.send(200, F("text/plain"), F("ON"));
}

void ledOFF() {
  digitalWrite(LED_BUILTIN, 0);
  server.send(200, F("text/plain"), F("OFF"));
}

// Logic for Movement
void moveForward() {
  digitalWrite(leftDIR, HIGH);
  digitalWrite(rightDIR, HIGH);
  digitalWrite(leftEN, HIGH);
  digitalWrite(rightEN, HIGH);
  server.send(200, F("text/plain"), F("FORWARD"));
}

void moveReverse() {
  digitalWrite(leftDIR, LOW);
  digitalWrite(rightDIR, LOW);
  digitalWrite(leftEN, HIGH);
  digitalWrite(rightEN, HIGH);
  server.send(200, F("text/plain"), F("REVERSE"));
}

void moveRight() {
  digitalWrite(leftDIR, LOW);
  digitalWrite(rightDIR, LOW);
  digitalWrite(leftEN, HIGH);
  digitalWrite(rightEN, HIGH);
  server.send(200, F("text/plain"), F("RIGHT"));
}

void moveLeft() {
  digitalWrite(leftDIR, LOW);
  digitalWrite(rightDIR, LOW);
  digitalWrite(leftEN, LOW);
  digitalWrite(rightEN, LOW);
  server.send(200, F("text/plain"), F("LEFT"));
}

void handleNotFound() {
  server.send(404, F("text/plain"), F("Not Found"));
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rightDIR, OUTPUT);
  pinMode(rightEN, OUTPUT);
  pinMode(leftDIR, OUTPUT);
  pinMode(leftEN, OUTPUT);

  Serial.begin(9600);
  while (!Serial && millis() < 1000);  

  if (WiFi.status() == WL_NO_SHIELD) {
    while (true);
  }

  if (groupNumber)
    WiFi.config(IPAddress(192, 168, 0, groupNumber + 1));

  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(500);
  }

  // REGISTER ALL CALLBACKS
  server.on(F("/"), handleRoot);
  server.on(F("/on"), ledON);
  server.on(F("/off"), ledOFF);
  server.on(F("/forward"), moveForward);
  server.on(F("/reverse"), moveReverse);
  server.on(F("/right"), moveRight);
  server.on(F("/left"), moveLeft);

  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();
}
