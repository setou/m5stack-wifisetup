#include <M5Stack.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include "WebServer.h"
#include <Preferences.h>

const IPAddress apIP(192, 168, 4, 1);
const char* apSSID = "M5STACK_SETUP";
const String apQrCodeStr = "WIFI:T:WPA;S:" + String(apSSID) + ";P:;;";

String ssidList;
String wifi_ssid;
String wifi_password;

DNSServer dnsServer;
WebServer webServer(80);

// wifi config store
Preferences preferences;

void setup() {
  m5.begin();
  M5.Lcd.setTextFont(2);
  preferences.begin("wifi-config");
  setupMode();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void startWebServer() {
  //  M5.Lcd.print("Starting Web Server at ");
  //  M5.Lcd.println(WiFi.softAPIP());
  webServer.on("/settings", []() {
    String s = "<h1>Wi-Fi Settings</h1><p>Please enter your password by selecting the SSID.</p>";
    s += "<form method=\"get\" action=\"setap\"><label>SSID: </label><select name=\"ssid\">";
    s += ssidList;
    s += "</select><br>Password: <input name=\"pass\" length=64 type=\"password\"><input type=\"submit\"></form>";
    webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
  });
  webServer.on("/setap", []() {
    String ssid = urlDecode(webServer.arg("ssid"));
    String pass = urlDecode(webServer.arg("pass"));

    // Store wifi config
    M5.Lcd.println("Writing SSID to EEPROM...");
    preferences.putString("WIFI_SSID", ssid);
    M5.Lcd.println("Writing Password to nvr...");
    preferences.putString("WIFI_PASSWD", pass);

    M5.Lcd.println("Write nvr done!");
    String s = "<h1>Setup complete.</h1><p>device will be connected to \"";
    s += ssid;
    s += "\" after the restart.";
    webServer.send(200, "text/html", makePage("Wi-Fi Settings", s));
    delay(3000);
    ESP.restart();
  });
  webServer.onNotFound([]() {
    String s = "<h1>AP mode</h1><p><a href=\"/settings\">Wi-Fi Settings</a></p>";
    webServer.send(200, "text/html", makePage("AP mode", s));
  });

  webServer.begin();
}

void setupMode() {
  WiFi.mode(WIFI_MODE_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  delay(100);
  for (int i = 0; i < n; ++i) {
    ssidList += "<option value=\"";
    ssidList += WiFi.SSID(i);
    ssidList += "\">";
    ssidList += WiFi.SSID(i);
    ssidList += "</option>";
  }
  delay(100);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apSSID);
  WiFi.mode(WIFI_MODE_AP);
  // WiFi.softAPConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
  // WiFi.softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0);
  dnsServer.start(53, "*", apIP);
  startWebServer();
  M5.Lcd.print("    Starting Access Point at \"");
  M5.Lcd.print(apSSID);
  M5.Lcd.println("\"");
  M5.Lcd.qrcode(apQrCodeStr, 60, 25, 200, 4);
}

String makePage(String title, String contents) {
  String s = "<!DOCTYPE html><html><head>";
  s += "<meta name=\"viewport\" content=\"width=device-width,user-scalable=0\">";
  s += "<title>";
  s += title;
  s += "</title></head><body>";
  s += contents;
  s += "</body></html>";
  return s;
}

String urlDecode(String input) {
  String s = input;
  s.replace("%20", " ");
  s.replace("+", " ");
  s.replace("%21", "!");
  s.replace("%22", "\"");
  s.replace("%23", "#");
  s.replace("%24", "$");
  s.replace("%25", "%");
  s.replace("%26", "&");
  s.replace("%27", "\'");
  s.replace("%28", "(");
  s.replace("%29", ")");
  s.replace("%30", "*");
  s.replace("%31", "+");
  s.replace("%2C", ",");
  s.replace("%2E", ".");
  s.replace("%2F", "/");
  s.replace("%2C", ",");
  s.replace("%3A", ":");
  s.replace("%3A", ";");
  s.replace("%3C", "<");
  s.replace("%3D", "=");
  s.replace("%3E", ">");
  s.replace("%3F", "?");
  s.replace("%40", "@");
  s.replace("%5B", "[");
  s.replace("%5C", "\\");
  s.replace("%5D", "]");
  s.replace("%5E", "^");
  s.replace("%5F", "-");
  s.replace("%60", "`");
  return s;
}
