/****Includes****/
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <Adafruit_NeoPixel.h>    //https://github.com/adafruit/Adafruit_NeoPixel
#include <elapsedMillis.h>        //https://github.com/pfeerick/elapsedMillis

bool leds_states[7] = {0};
elapsedMillis elapsedTime;

/****Config****/
#define PIN   D1
const char *ssid = "River";

/*****Initialization*****/
ESP8266WebServer server(80);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(7, PIN, NEO_GRB + NEO_KHZ800);

/*****WebPage*****/
void handleRoot() {
  server.send(200, "text/html", "\
<!doctype html>\
<html>\
<head>\
  <title>Paulette' Necklace</title>\
  <link rel='stylesheet' href='http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.css'>\
  <script src='http://code.jquery.com/jquery-1.11.3.min.js'></script>\
  <script src='http://code.jquery.com/mobile/1.4.5/jquery.mobile-1.4.5.min.js'></script>\
  <style>\
    .LED\
    {\
        width:100%;\
        border: 2px solid black;\
        border:2px solid black;\
    }\
  </style>\
</head>\
<body>\
    <span></span>\
    <div id='0' class='LED'>LED 0</div>\
    <div id='1' class='LED'>LED 1</div>\
    <div id='2' class='LED'>LED 2</div>\
    <div id='3' class='LED'>LED 3</div>\
    <div id='4' class='LED'>LED 4</div>\
    <div id='5' class='LED'>LED 5</div>\
    <div id='6' class='LED'>LED 6</div>\
    <script type='text/javascript' charset='utf-8'>\
      $(document).ready(function(){\
        var w = $(window).width();\
        var h = $(window).height();\
        var ledButton = $('div.LED');\
        ledButton.each(function( index ) {\
          $( this ).css('height',h/ledButton.length);\
        });\
        ledButton.on('swipe',function(){\
          $(this).toggleClass('on');\
          var domElement = $(this).get(0);\
          $( 'span:first' ).text( 'swipe on - ' + domElement.className + ' ' + domElement.id);\
          $.get( '/settings' , {id:domElement.id, state:domElement.className});\
        });\
      });\
    </script>\
</body>\
</html>\
");
}

/****Manage LEDs****/
void handleLEDs() {
  if ( server.hasArg("id") && server.hasArg("state") ) {
    int id = server.arg("id").toInt();

    // check if state changed:
    if (leds_states[id] != server.arg("state").equals("LED on")) {
      leds_states[id] = server.arg("state").equals("LED on");

      Serial.print("id: ");
      Serial.print( id );
      Serial.print(" - state: ");
      Serial.println( leds_states[id] );
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

void setupPixels() {
  // This initializes the NeoPixel library.
  pixels.begin();
}

void setup() {
  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);

  setupSerial();
  setupWifi();
  setupServer();
  setupPixels();
}

/****Loop****/
void loop() {
  server.handleClient();
  nonBlockingRainbow(20);
}

/****Neopixels****/
void nonBlockingRainbow(int waitMs) {

  // non blocking delay:
  if (elapsedTime < waitMs)
    return;
  elapsedTime = 0;
  digitalWrite(D4, !digitalRead(D4)); // debug blink

  // "loop" on colors:
  static int j = 0;
  j = j < 256 ? j+1 : 0;

  // "loop" on pixels:
  for(int i=0; i<pixels.numPixels(); i++) {
    if (leds_states[i]) // is pixel web-enabled?
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    else
      pixels.setPixelColor(i, 0); // off
  }

  pixels.show();
}

uint32_t Wheel(byte WheelPos) {
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

