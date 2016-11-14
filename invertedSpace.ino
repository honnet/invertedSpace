/****Includes****/
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <Adafruit_NeoPixel.h>    //https://github.com/adafruit/Adafruit_NeoPixel
#include <elapsedMillis.h>        //https://github.com/pfeerick/elapsedMillis
#include <ESP8266mDNS.h>          //Allow custom URL
#include "handleRootHTML.h"       //Contains the HTML minified from index.html see README.md

bool leds_states[7] = {1,1,1,1,1,1,1};
int r[7] = {0,0,0,0,0,0,0};
int g[7] = {0,0,0,0,0,0,0};
int b[7] = {0,0,0,0,0,0,0};
elapsedMillis elapsedTime;

/****Config****/
#define  LED_STRIP_PIN  D1 /* LED STRIP PIN - AKA GPIO5 on witty modules */
const char *ssid = "invertedSpace";

/*****Initialization*****/
ESP8266WebServer server(80);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(7, LED_STRIP_PIN, NEO_GRB + NEO_KHZ800);

// Content in handleRootHTML.h, see README for details:
extern String handleRootHTML;

/*****WebPage*****/
void handleRoot() {
  server.send(200, "text/html", handleRootHTML);
}

/****Manage LEDs****/
void handleLEDs() {
  if ( server.hasArg("id") && server.hasArg("state") && server.hasArg("r") ) {
    int id = server.arg("id").toInt();
    // check if state changed:
    if (leds_states[id] != server.arg("state").equals("1")) {
      leds_states[id] = server.arg("state").equals("1");
      r[id] = server.arg("r").toInt();
      g[id] = server.arg("g").toInt();
      b[id] = server.arg("b").toInt();
    }
  }
  else {
    Serial.println("Bad URL.");
  }
}

/****Setups****/
void setupSerial() {
  delay(1000);
  Serial.begin(115200);
}

void setupWifi() {
  //WiFiManager
  WiFiManager wifiManager;

  //reset saved settings -- Flush flash
  //wifiManager.resetSettings();
  //fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(ssid);

  // might seem redundant but it's not printed the 1st time:
  Serial.println("local ip");
  Serial.println(WiFi.localIP());
}

void setupServer() {
  server.on("/", handleRoot);
  server.on("/settings", handleLEDs);
  server.begin();
  Serial.println("HTTP server started");
}

void setupMDNS() {
  // Add service to MDNS-SD to access the ESP with the URL http://<ssid>.local
  if (MDNS.begin(ssid)) {
    Serial.print("MDNS responder started as http://");
    Serial.print(ssid);
    Serial.println(".local");
  }
  MDNS.addService("http", "tcp", 8080);
}

void setupPixels() {
  // This initializes the NeoPixel library.
  pixels.begin();
  nonBlockingRainbow(0); // ...and turn on LEDs in the strip
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW); // = ON (inverted logic)

  setupSerial();

  Serial.println("Starting LEDs.");
  setupPixels();

  Serial.println("Starting WiFi.");
  setupWifi();
  setupServer();
  setupMDNS();

  Serial.println("Setup OK.");
}

/****Loop****/
void loop() {
  nonBlockingRainbow(20);
  server.handleClient();
}

/****Neopixels****/
void nonBlockingRainbow(int waitMs) {

  // non blocking delay:
  if (elapsedTime < waitMs)
    return;
  elapsedTime = 0;
  digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED)); // debug blink

  // "loop" on colors:
  static int j = 0;
  j = j < 256 ? j+1 : 0;

  // "loop" on pixels:
  for(int i=0; i<pixels.numPixels(); i++) {
    if (leds_states[i]) // is pixel web-enabled?
      //pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
      pixels.setPixelColor(i, pixels.Color(r[i],g[i],b[i]));
    else
      pixels.setPixelColor(i, 0); // off
  }

  pixels.show();
}

uint32_t Wheel(byte WheelPos) {
  // color range:
  const int violet = 210;
  const int yellow = 30;

  // convertion:
  const int range = yellow + 255 - violet;
  WheelPos = ( range * WheelPos / 255 + violet ) % 255;

  // normal wheel:
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

