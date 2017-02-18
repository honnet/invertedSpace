# Inverted Space

IoT & Art!?

In this project we control LEDs from a web interface (using ESP8266 and WS2812).￼
The applications range from air polution monitoring to simple clock, or artistic visualizations, check this repo:

https://github.com/GeorgeGally/blacklight

Made in China - 2016 ([HTTC](http://noisebridge.net/wiki/NoisebridgeChinaTrip6) extension).

<img src="https://pbs.twimg.com/media/CyjXnRDWEAAyjtL.jpg" alt="latest test" width="300"/>
<img src="https://pbs.twimg.com/media/Cx-CuLdXABE3LTK.jpg" alt="first test" width="300"/>

## Dependencies

ESP8266 Arduino:

    https://github.com/esp8266/Arduino

WiFiManager - WiFi Configuration Magic:

    https://github.com/tzapu/WiFiManager

Adafruit NeoPixel:

    https://github.com/adafruit/Adafruit_NeoPixel

Elapsed milliseconds:

    https://github.com/pfeerick/elapsedMillis


## Usage

1st time:
  * program the ESP with the arduino IDE (the 1st link should have every details needed)
  * connect your phone/computer to your the wifi network called 'invertedSpace'
  * go to the following URL with your browser: http://192.168.4.1/
  * set the ESP to connect to your favorite network (set the SSID and password)

Then:
  * your phone/computer should connect back to your favorite network
  * put the following URL in your browser, the rest should be intuitive:

    http://invertedSpace.local/

Note: If you need to change the password, uncomment the follwing line in the .ino file:

    wifiManager.resetSettings();

## Advanced

The HTML/JS code is embedded in the arduino code, using the handleRootHTML.h header.
To update it, edit index.html and use the following command to update the header:

    ./minify.sh

You will need html-minifier accessible with node, to install it:

    npm install html-minifier -g

