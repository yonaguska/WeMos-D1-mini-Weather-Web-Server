Weather server on Wemos D1 mini
==================================================================================
Initially I was going to implement a project I saw on the web, but that seemed too simple and limited. I added code that avoids having to embed your SSID and Password in the code. I augmented the code to include an API call to a weather service that gave me my loacal sea level pressure, from which I derived my altitude. I added code to map the wind direction to compass ordinals. There's more I want to do, like upload the data to either a web service or my own InfluxDB so I can accumulate the readings and plot them on a web page.



I have constructed a small Weather Web Server based on a Wemos D2 mini, an OLED display and a BME280 sensor. It was programmed using the Arduino IDE and implements a collection of capabilities I borrowed from several other projects I found around the web. Features include:
<LI>Connectivity to your home Wifi to publish a web page containing data collected by the program</LI>
<LI>Date and Time from NIST</LI>
<LI>emperature, Humidity and Barometric pressure from the BME280 sensor</LI>
<LI>Dew point and altitude from a library for the BME280</LI>
<LI>Wind speed, direction and map co-ordinates from an API to a service</LI> 
<LI>Mapping of the wind direction to one of the 32 compass ordinals</LI>


Parts list from Aliexpress (eBay will also have these)
==================================================================================

<A HREF="https://www.aliexpress.com/item/ESP8266-ESP-12-USB-WeMos-D1-Mini-WIFI-Development-Board-D1-Mini-NodeMCU-Lua-IOT-Board/32829531277.html?spm=2114.search0204.3.18.mkljsH&ws_ab_test=searchweb0_0,searchweb201602_4_10152_10065_10151_10130_5490020_10068_5470017_5560011_10307_10137_10060_10155_10154_10056_10055_10054_10059_100031_10099_5460020_10338_10103_10102_440_10052_10053_10107_10050_10142_10051_10324_10325_5380020_10326_10084_513_10083_10080_10082_10081_10178_10110_10111_10112_10113_10114_143_5570011_10312_10313_10314_10078_10079_10073_5550017,searchweb201603_18,ppcSwitch_4_ppcChannel&btsid=695e591f-432a-4222-9c73-e8f40b67ca57&algo_expid=a0bc7d45-b95f-4f0d-9695-db6623a85ef1-2&algo_pvid=a0bc7d45-b95f-4f0d-9695-db6623a85ef1">Wemos D1 mini</A>

<A HREF="https://www.aliexpress.com/item/20pcs-Free-Shipping-White-Blue-White-and-Blue-color-0-96-inch-128X64-OLED-Display-Module/32727927917.html?spm=a2g0s.9042311.0.0.C9chG9">Blue OLED display</A>

<A HREP="https://www.aliexpress.com/item/Free-shipping-BME280-Digital-Sensor-Temperature-Humidity-Barometric-Pressure-Sensor-Module-GY-BME280-I2C-SPI-1/32829699334.html?spm=2114.search0204.3.150.EysWqm&ws_ab_test=searchweb0_0,searchweb201602_4_10152_10065_10151_10130_5490020_10068_5560011_5550020_10307_10137_10060_10155_10154_10056_10055_10054_5470020_10059_100031_10099_5460020_10338_10103_10102_440_10052_10053_10107_10050_10142_10051_10324_10325_5380020_10326_10084_513_10083_10080_10082_10081_10178_10110_10111_10112_10113_10114_143_5570011_10312_10313_10314_10078_10079_10073,searchweb201603_18,ppcSwitch_4_ppcChannel&btsid=5b876f03-631a-4cfa-b493-6cef54d45bf8&algo_expid=5fe2def1-214e-40b3-952a-2bfee48ca0e5-20&algo_pvid=5fe2def1-214e-40b3-952a-2bfee48ca0e5">BME280 Digital Sensor - temperature, humidity and barometric pressure</A>

Micro USB cable, for programming and power
https://www.aliexpress.com/item/Suntaiho-NEW-Nylon-Micro-USB-Cable-Fast-Charging-Adapter-5V2-1A-8pin-25CM-1M-2M-3M/32696694333.html?spm=2114.search0204.3.40.WQ30bz&s=p&ws_ab_test=searchweb0_0,searchweb201602_4_10152_10065_10151_10130_5490020_10068_5470017_5560011_10307_10137_10060_10155_10154_10056_10055_10054_10059_100031_10099_5460020_10338_10103_10102_440_10052_10053_10107_10050_10142_10051_10324_10325_5380020_10326_10084_513_10083_10080_10082_10081_10178_10110_10111_10112_10113_10114_143_5570011_10312_10313_10314_10078_10079_10073_5550017,searchweb201603_18,ppcSwitch_4_ppcChannel&btsid=893ddb45-8d08-4b4c-9f67-438f3e352d7f

Project details:
==================================================================================

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

WeMos D1 mini documentation (pinout)
https://wiki.wemos.cc/products:d1:d1_mini

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







