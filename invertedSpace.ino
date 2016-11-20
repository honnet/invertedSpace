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

typedef enum color_e {RED, YELLOW, GREEN} color_t;
bool leds_states[7] = {1,1,1,1,1,1,1};
int r[7] = {9,9,9,9,9,9,9};
int g[7] = {9,9,9,9,9,9,9};
int b[7] = {9,9,9,9,9,9,9};
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
  if ( server.hasArg("r") && server.hasArg("g") && server.hasArg("b") &&
       server.hasArg("id") && server.hasArg("state") ) {
    int id = server.arg("id").toInt();
    // check if state changed:
    if (leds_states[id] != server.arg("state").equals("1")) {
      leds_states[id] = server.arg("state").equals("1");
      r[id] = server.arg("r").toInt();
      g[id] = server.arg("g").toInt();
      b[id] = server.arg("b").toInt();
    }
    server.send(200, "text/plain", "");
  }
  else {
    Serial.println("Bad URL.");
    server.send(404, "text/plain", "Bad URL.");
  }
}

/****Setups****/
void setupSerial() {
  delay(1000);
  Serial.begin(115200);
}

//gets called when WiFiManager enters configuration mode
void configModeCallback(WiFiManager *myWiFiManager) {
  LEDfeedback(RED); // state feedback
}

void setupWifi() {
  //WiFiManager
  WiFiManager wifiManager;

  //reset saved settings -- Flush flash
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

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
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW); // = ON (inverted logic)

  setupSerial();

  Serial.println("Starting LEDs.");
  setupPixels();
  LEDfeedback(YELLOW); // state feedback

  Serial.println("Starting WiFi.");
  setupWifi();
  setupServer();
  setupMDNS();
  LEDfeedback(GREEN); // state feedback

  Serial.println("Setup OK.");
}

/****Loop****/
void loop() {
  nonBlockingLEDcontrol(20);
  server.handleClient();
}

/****Neopixels****/
void nonBlockingLEDcontrol(int waitMs) {

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
      pixels.setPixelColor(i, pixels.Color(r[i],g[i],b[i]));
    else
      pixels.setPixelColor(i, 0); // off
  }

  pixels.show();
}

void LEDfeedback(color_t color) {

  int R=0, G=0, B=0;

  switch (color) {
    case YELLOW :
      R = 255;
      G = 255;
      break;
    case RED :
      R = 255;
      break;
    case GREEN :
      G = 255;
      break;
  }

  // "loop" on pixels:
  for(int i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(R,G,B));
  }

  pixels.show();
}

