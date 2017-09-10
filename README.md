# WeMos-D1-mini-Weather-Web-Server

Here are my notes colelcted through the course of creating this Weather Web Server

The core of the project is here:
http://embedded-lab.com/blog/making-simple-weather-web-server-using-esp8266-bme280/

Drivers for NodeMCU and ESPs (for Windows, Mac and Linux):
http://www.14core.com/drivers/
https://www.14core.com/installing-nodemcu-driver-on-mac/ (the Mac page)

Adafruit Libraries and usage:
https://github.com/adafruit/Adafruit_Sensor
https://github.com/adafruit/Adafruit_BME280_Library
https://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use/how-to-install-a-library
https://learn.adafruit.com/adafruit-bme280-humidity-barometric-pressure-temperature-sensor-breakout
https://learn.adafruit.com/adafruit-bmp280-barometric-pressure-plus-temperature-sensor-breakout/wiring-and-test

NodeMCU’s documentation for BME280 (core reference, Adafriut’s is a little different):
https://nodemcu.readthedocs.io/en/master/en/modules/bme280/

NodeMCU’s IIC documentation (fyi only):
http://nodemcu.readthedocs.io/en/master/en/modules/i2c/

Time Functions:
http://www.geekstips.com/arduino-time-sync-ntp-server-esp8266-udp/
https://www.pjrc.com/teensy/td_libs_Time.html


An IDE (not Arduino) for ESP development; I used Arduino:
http://www.14core.com/how-to-install-esplorer-ide-in-multiple-platform/

I can’t seem to flash my ESP-01s, that’s why I used a NodeMCU, it’s easier:
https://www.letscontrolit.com/forum/viewtopic.php?f=5&t=2780

Addition of the OLED
http://embedded-lab.com/blog/tutorial-6-esp8266-bme280-make-localremote-weather-station/ 

Making the OLED work…it’s at address 0x3C, change it in the library…and the code. 
I discovered I had made a mistake, using the wrong address and a minor wiring error (reversed the clock and data lines)
http://www.instructables.com/id/Monochrome-096-i2c-OLED-display-with-arduino-SSD13/

Adding OpenWeatherMap.org connectivity
http://educ8s.tv/esp8266-weather-display/ this gave me the idea
http://openweathermap.org/appid this is the OpenWeatherMap web site
http://openweathermap.org/current this is how to get weather from one location (using city ID)
    An example of the API call you can put into your browser (you have to subscribe to the service to get your APPID and get a list of the cityIDs)
    http://api.openweathermap.org/data/2.5/weather?id=YourCityIDGoesHere&APPID=YourOpenWeatherAPPIDGoesHere

Decoding the JSON response of OpenWeatherMap is easier to understand using this little web app
You paste in the response (from a browser that doesn’t format it) and it shows you the code required for pulling out the data
https://bblanchon.github.io/ArduinoJson/assistant/

To avoid hardcoding the SSID and password, I used the WiFiManager
http://www.instructables.com/id/Avoid-Hard-Coding-WiFi-Credentials-on-Your-ESP8266/

We mapped the wind direction to compass ordinals
https://images.search.yahoo.com/yhs/search;_ylt=A0LEViq1qbVZ354ATuwPxQt.?p=compass+ordinals&fr=yhs-adk-adk_sbnt&fr2=piv-web&hspart=adk&hsimp=yhs-adk_sbnt&type=we_appfocus1_cr#id=6&iurl=http%3A%2F%2F1.bp.blogspot.com%2F-QhDomqp-KVs%2FU_dma4AnG9I%2FAAAAAAAAOhU%2F-NsWTzOPGF4%2Fs1600%2F210A-Image%252BCompass%252BPoints.jpg&action=click

You should be able to use my ino file and simply tweak the OpenWeatherMap CityID and APPID values to get your node running.
