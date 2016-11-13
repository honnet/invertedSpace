/****Includes****/
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiClient.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <Adafruit_NeoPixel.h>    //https://github.com/adafruit/Adafruit_NeoPixel
#include <elapsedMillis.h>        //https://github.com/pfeerick/elapsedMillis
#include <ESP8266mDNS.h>          //Allow custom URL

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

/*****WebPage*****/
void handleRoot() {
  String html = "<!DOCTYPE html><title>Blacklight</title><style>body{background:#f7f7f7;font-family:helvetica,arial,sans}.holder{margin:0 auto 0 auto}.led{width:80%;box-shadow:0 1px 1px #ccc;padding:10px;margin:6px auto 4px auto;background:#fff;text-align:center}.led:active,.led:hover{background:#e2674a}.led:hover .on,.on{background:#39c;color:#fff}</style><div class=holder><div class='led on'id=0>LED 0</div><div class='led on'id=1>LED 1</div><div class='led on'id=2>LED 2</div><div class='led on'id=3>LED 3</div><div class='led on'id=4>LED 4</div><div class='led on'id=5>LED 5</div><div class='led on'id=6>LED 6</div></div><script>function lightup(n){var t=n.target.id,e=document.getElementById(t);e.classList.toggle('on');var o=0,a=Math.round(255*Math.random()),c=Math.round(255*Math.random()),s=Math.round(255*Math.random()),i=a+','+c+','+s;if(e.classList.contains('on')){var o=1;e.style.background='rgb('+i+')'}else e.style.background='#f7f7f7';var l='/settings?id='+t+'&state='+o+'&r='+a+'&g='+c+'&b='+s;console.log(l),$jsonp.send(l,{callbackName:'dataSent',onSuccess:function(n){console.log('success!',n)},onTimeout:function(){console.log('timeout!')},timeout:5})}function dataSent(n){console.log(n)}var leds=document.getElementsByClassName('led');for(i=0;i<leds.length;i++)leds[i].addEventListener('click',lightup,!1);var $jsonp=function(){var n={};return n.send=function(n,t){var e=t.callbackName||'callback',o=t.onSuccess||function(){},a=t.onTimeout||function(){},c=t.timeout||10,s=window.setTimeout(function(){window[e]=function(){},a()},1e3*c);window[e]=function(n){window.clearTimeout(s),o(n)};var i=document.createElement('script');i.type='text/javascript',i.async=!0,i.src=n,document.getElementsByTagName('head')[0].appendChild(i)},n}()</script>";
  server.send(200, "text/html", html);
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

