// ESP8266 Smart Alarm Clock with MAX7219 LED Matrix
// Features:
// - 12-hour time display using NTP
// - Multiple alarms via buzzer, stays ON until turned off from web
// - Alarm time saved in EEPROM
// - Web interface to set/disable alarm and display custom text
// - Custom text overrides time/date, scrolls if long
// - Each alarm can be enabled/disabled/stopped independently

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <TimeLib.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <SPI.h>
#include <ESP8266mDNS.h> 



// Wi-Fi Credentials
const char* ssid = "bsnl-1392";
const char* password = "bsnl@68319";

// LED Matrix Setup
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN D8
MD_Parola display = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// NTP Time Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST

// Web Server
ESP8266WebServer server(80);

// Buzzer Pin
#define BUZZER_PIN D1

// EEPROM Storage
#define EEPROM_SIZE 256
struct Alarm {
  int hour;
  int minute;
  bool pm;
  bool enabled;
};

Alarm alarms[3];
bool alarmActive[3] = {false, false, false};
bool alarmTriggered[3] = {false, false, false};
bool showTime = true;
unsigned long lastToggle = 0;
char customText[32] = "";

void saveSettings() {
  int addr = 0;
  for (int i = 0; i < 3; i++) {
    EEPROM.write(addr++, alarms[i].hour);
    EEPROM.write(addr++, alarms[i].minute);
    EEPROM.write(addr++, alarms[i].pm);
    EEPROM.write(addr++, alarms[i].enabled);
  }
  for (int i = 0; i < 32; i++) EEPROM.write(addr++, customText[i]);
  EEPROM.commit();
}

void loadSettings() {
  int addr = 0;
  for (int i = 0; i < 3; i++) {
    alarms[i].hour = EEPROM.read(addr++);
    alarms[i].minute = EEPROM.read(addr++);
    alarms[i].pm = EEPROM.read(addr++);
    alarms[i].enabled = EEPROM.read(addr++);
  }
  for (int i = 0; i < 32; i++) customText[i] = EEPROM.read(addr++);
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Alarm Clock</title><style>body{font-family:sans-serif;background:#222;color:#eee;text-align:center;padding:20px;}input,select{margin:5px;padding:6px;}button,input[type=submit]{padding:8px 16px;background:#08f;color:#fff;border:none;cursor:pointer;}form{margin:10px auto;width:90%;max-width:300px;border:1px solid #444;padding:10px;border-radius:8px;background:#333;}h2{color:#0f0;}</style></head><body><h2>ESP8266 Alarm Clock</h2>";

  html += "<form action='/set' method='GET'><h3>Set Alarm</h3>Index:<select name='idx'><option value='0'>1</option><option value='1'>2</option><option value='2'>3</option></select><br>Hour:<input type='number' name='hr' min='1' max='12' required><br>Minute:<input type='number' name='min' min='0' max='59' required><br>AM/PM:<select name='ampm'><option value='AM'>AM</option><option value='PM'>PM</option></select><br><input type='submit' value='Set Alarm'></form>";

  html += "<form action='/text' method='GET'><h3>Custom Text</h3><input type='text' name='msg' maxlength='30' placeholder='Enter text'><br><input type='submit' value='Set Text'></form>";

  for (int i = 0; i < 3; i++) {
    if (alarmActive[i]) {
      html += "<form action='/stop?idx=" + String(i) + "'><input type='submit' value='Stop Alarm " + String(i + 1) + "'></form>";
    }
  }

  html += "<form action='/disable'><input type='submit' value='Disable All Alarms'></form><br><p><b>Alarms:</b><br>";
  for (int i = 0; i < 3; i++) {
    if (alarms[i].enabled) {
      html += String(i + 1) + ": ";
      html += alarms[i].hour;
      html += ":";
      if (alarms[i].minute < 10) html += "0";
      html += alarms[i].minute;
      html += alarms[i].pm ? " PM" : " AM";
      html += "<br>";
    }
  }
  html += "</p></body></html>";
  server.send(200, "text/html", html);
}

void handleSetAlarm() {
  int idx = server.arg("idx").toInt();
  alarms[idx].hour = server.arg("hr").toInt();
  alarms[idx].minute = server.arg("min").toInt();
  alarms[idx].pm = server.arg("ampm") == "PM";
  alarms[idx].enabled = true;
  saveSettings();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleSetText() {
  String msg = server.arg("msg");
  msg.toCharArray(customText, sizeof(customText));
  saveSettings();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleStopAlarm() {
  if (server.hasArg("idx")) {
    int idx = server.arg("idx").toInt();
    alarmActive[idx] = false;
    digitalWrite(BUZZER_PIN, LOW);
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleDisableAlarm() {
  for (int i = 0; i < 3; i++) alarms[i].enabled = false;
  for (int i = 0; i < 3; i++) alarmActive[i] = false;
  digitalWrite(BUZZER_PIN, LOW);
  saveSettings();
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

  display.begin();
  display.setIntensity(5);
  display.displayClear();
  display.setTextAlignment(PA_CENTER);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  timeClient.begin();

Serial.println();
Serial.print("ESP8266 IP address: ");
Serial.println(WiFi.localIP());

// Show IP on MAX7219 matrix
display.displayClear();
display.setTextAlignment(PA_LEFT);

// Convert IP to string and scroll it
String ipText = "IP: " + WiFi.localIP().toString();
display.displayScroll(ipText.c_str(), PA_LEFT, PA_SCROLL_LEFT, 80);

// Animate scrolling
while (!display.displayAnimate()) {
  delay(10);
}
if (!MDNS.begin("espclock")) {  // Use a hostname like espclock
  Serial.println("Error starting mDNS");
  while (true);
}
Serial.println("mDNS started: http://espclock.local");




  server.on("/", handleRoot);
  server.on("/set", handleSetAlarm);
  server.on("/text", handleSetText);
  server.on("/stop", handleStopAlarm);
  server.on("/disable", handleDisableAlarm);
  server.begin();
}

void loop() {
  server.handleClient();
  timeClient.update();

  time_t now = timeClient.getEpochTime();
  int hr24 = hour(now);
  int min = minute(now);
  int hr12 = hr24 % 12; if (hr12 == 0) hr12 = 12;
  bool isPM = hr24 >= 12;
  int d = day(now); int m = month(now);

  bool anyAlarmActive = false;
  for (int i = 0; i < 3; i++) {
    if (alarms[i].enabled && hr12 == alarms[i].hour && min == alarms[i].minute && isPM == alarms[i].pm && !alarmTriggered[i]) {
      digitalWrite(BUZZER_PIN, HIGH);
      alarmActive[i] = true;
      alarmTriggered[i] = true;
    }
    if (min != alarms[i].minute) alarmTriggered[i] = false;
    if (alarmActive[i]) anyAlarmActive = true;
  }
  if (!anyAlarmActive) digitalWrite(BUZZER_PIN, LOW);

  if (strlen(customText) > 0 && !anyAlarmActive) {
    display.displayClear();
    display.setTextAlignment(PA_LEFT);
    display.displayScroll(customText, PA_LEFT, PA_SCROLL_LEFT, 100);
    while (!display.displayAnimate()) {
      server.handleClient();
      delay(10);
    }
    return;
  }

  unsigned long nowMillis = millis();
  if (nowMillis - lastToggle > 5000) {
    showTime = !showTime;
    lastToggle = nowMillis;
    display.displayClear();
  }

  char buffer[40];
  if (showTime) {
    sprintf(buffer, "%02d%c%02d", hr12, (millis()/500)%2 ? ':' : ' ', min);
  } else {
    sprintf(buffer, "%02d %s", d, monthShortStr(m));
  }

  display.setTextAlignment(PA_CENTER);
  display.print(buffer);
  delay(100);
}
