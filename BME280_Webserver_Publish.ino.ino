/*********
  Project: BME Weather Web server using Wemos Node Mini
  Implements Adafruit's sensor libraries.
  Complete project is at: http://embedded-lab.com/blog/making-a-simple-weather-web-server-using-esp8266-and-bme280/
  
  Modified code from Rui Santos' Temperature Weather Server posted on http://randomnerdtutorials.com

  Added altitude, nodeType and debug flag

  Added OLED display

  Added WifiManager to configure the wifi settings

  Added API call to get sea level pressure for altitude calculation
  
*********/

#include <Wire.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include <TimeLib.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include <ArduinoJson.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
//#include <gfxfont.h>

Adafruit_BME280 bme; // I2C

char debug = 1; // toggles showing: board type and altitude information, or generic ESP8266 and no altitude
float myAltitudeInMeters = 196.0; // Nevada, TX
float mySeaLevelPressure = 0.0;
//const char* nodeType = "ESP8266";
const char* nodeType = "Wemos D1 Mini with BME280 - ";
//const char* nodeType = "NodeMCU";

float h, t, p, pin, dp, a, af;
char temperatureFString[6];
char dpString[6];
char humidityString[6];
char pressureString[7];
char pressureDifferenceString[7];
char pressureInchString[6];
char altitudeMetersString[6];
char altitudeFeetString[6];
char timeDateAndTimeString[40];
char dateString[11];
char timeString[9];
int  pressureArray[100];
int  pressureArrayMax = 100;
int  pressureSamples = 0;
int  pressureArrayIndex = 0;
char pressureTrend[10];
float pressureAverage = 0.0;
float pressureTotal = 0.0;
float pressureDifference = 0.0;
char myOLEDString[50];
unsigned long secsSince1900;
unsigned long lastWeatherPoll = 0L;    // so we don't exceed the polling limit of OpenWeatherMap
unsigned long fiveMinutes = 60L * 5L;  // we'll poll every five minutes

// OpenWeatherMap stuff -------------------------------------------------------------------
String CityID = "yourCityIDgoesHere";                // City code for your city ... from OpenWeatherMap
String APIKEY = "yourAPPIDgoesHere";                 // ...and your API key from OpenWeatherMap
float coord_lon;                                     // we'll get that from the API
float coord_lat;                                     // we'll get that from the API
int weather0_id;                                     // we'll get that from the API
const char* weather0_main;                           // we'll get that from the API
const char* weather0_description;                    // we'll get that from the API
const char* weather0_icon;                           // we'll get that from the API
const char* base;                                    // we'll get that from the API
float main_temp;                                     // we'll get that from the API
int main_pressure;                                   // we'll get that from the API
int main_humidity;                                   // we'll get that from the API
float main_temp_min;                                 // we'll get that from the API
float main_temp_max;                                 // we'll get that from the API
int visibility;                                      // we'll get that from the API
float wind_speed;                                    // we'll get that from the API m/s
int wind_deg;                                        // we'll get that from the API
const char* windDirection;
float wind_gust;                                     // we'll get that from the API m/s
int clouds_all;                                      // we'll get that from the API
long dt;                                             // we'll get that from the API
int sys_type;                                        // we'll get that from the API
int sys_id;                                          // we'll get that from the API
float sys_message;                                   // we'll get that from the API
const char* sys_country;                             // we'll get that from the API
long sys_sunrise;                                    // we'll get that from the API
long sys_sunset;                                     // we'll get that from the API
const char* cityName;                                // we'll get that from the API

// These are used to map the wind direction to compass ordinals
// Uncomment the pair you want to use, and comment out the others
// ...for the simple 8 ordinals
//int ords = 8;
//char ord[8][3] = {"N", "NW", "W", "SW", "S", "SE", "E", "NE"};
// ...for 16 ordinals
//int ords = 16;
//char ord[16][4] = {"N", "NNW", "NW", "WNW", "W", "WSW", "SW", "SSW", "S", "SSE", "SE", "ESE", "E", "ENE", "NE", "NNE"};
// ...we wanted all 32 ordinals
int ords = 32;
char ord[32][8] = {"N", "N by W", "NNW", "NW by N", "NW", "NW by W", "WNW", "W by N", "W", "W by S", "WSW", "SW by W", "SW", "SW by S", "SSW", "S by W", "S", "S by E", "SSE", "SE by S", "SE", "SE by E", "ESE", "E by S", "E", "E by N", "ENE", "NE by E", "NE", "NE by N", "NNE", "N by E"};

const char* where; // used in mapping wind direction to compass ordinal

WiFiClient client;
char servername[]="api.openweathermap.org";  // remote server we will connect to
String result;

const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(12) + 410;
DynamicJsonBuffer jsonBuffer(bufferSize);
const char* json = "{\"coord\":{\"lon\":-96.43,\"lat\":33.03},\"weather\":[{\"id\":800,\"main\":\"Clear\",\"description\":\"clear sky\",\"icon\":\"01d\"}],\"base\":\"stations\",\"main\":{\"temp\":305.55,\"pressure\":1017,\"humidity\":49,\"temp_min\":305.15,\"temp_max\":306.15},\"visibility\":16093,\"wind\":{\"speed\":5.1,\"deg\":190,\"gust\":9.3},\"clouds\":{\"all\":1},\"dt\":1504547700,\"sys\":{\"type\":1,\"id\":2678,\"message\":0.0039,\"country\":\"US\",\"sunrise\":1504526570,\"sunset\":1504572320},\"id\":4705584,\"name\":\"Lavon\",\"cod\":200}";
//const char* json;


// Web Server on port 80
WiFiServer server(80);

// local port to listen for UDP packets
unsigned int localPort = 2390;

/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
//IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

// OLED Block
#define OLED_RESET 0
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void writeTextToOLED(int size, int cursorX, int cursorY, boolean setColor, char textString[]) {
  if (setColor) {
    display.setTextColor(WHITE);
  } else {
    display.setTextColor(BLACK);
  }
  // Now display some of my stuff
  display.setTextSize(size);
  display.setCursor(cursorX,cursorY);
  display.print(textString);
  display.display();
}

char* ip2CharArray(IPAddress ip) {
  static char a[16];
  sprintf(a, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  return a;
}

char* deg2dir(int d) {
   int index;
   int slice;

   Serial.print("On entry to deg2dir the value of d is: ");
   Serial.println(d);   
   slice = 360 / ords;   // degree span of each ord
   if (d == 0) {
      Serial.print("In deg2dir the altered value of d is: ");
      Serial.println(d);
      return &ord[0][0];
   }
 
   index = d / slice;
   Serial.print("On normal exit from deg2dir the value of d is: ");
   Serial.println(d);
   Serial.print("On normal exit from deg2dir the value of index is: ");
   Serial.println(index);
   Serial.print("On normal exit fromo deg2dir the value of dir is: ");
   Serial.println(ord[index][0]);
   return  &ord[index][0];
}

bool validString(const char* unknown) {
   int i = 0;
   int j = 0;
   int num_letters = 0;
   int num_digits = 0;

   while(unknown[i] != '\0') {
      j++; // increment char count
      if(isalpha(unknown[i])) {
         num_letters++;             /* Increment letter count     */
      }

      if(isdigit(unknown[i++])) {
         num_digits++;              /* Increment digit count      */
      }
   }
   if (j > (num_letters + num_digits + 2)) {
      return false;
   } else {
      return true;
   }
}

// only runs once on boot
void setup() {
  // Initializing serial port for debugging purposes
  Serial.begin(115200);
  delay(10);
  //Wire.begin(0, 2); // GPIO 0, 2
  Wire.begin(4, 5); // GPIO SDA, SCL
  Wire.setClock(100000);

  // Setup for WifiManager
  WiFiManager wifiManager;
  
  //first parameter is name of access point, second is the password
  wifiManager.autoConnect("Pooka", "Harvey");
  //if you get here you have connected to the WiFi
  Serial.println("connected...to wifi :)");
  
  // Starting the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);
  
  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
  char * myIP = ip2CharArray(WiFi.localIP());
  
  //BME init
  Serial.println(F("BME280 test"));
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  // OLED Initialization
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64)
  // init done
  Serial.println("Initialize OLED with Adafruit splash");
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  
  // Now display some of my stuff, size 2
  display.clearDisplay();
  writeTextToOLED(2,0,16,1,"Yonaguska");
  delay(2000);
  //writeTextToOLED(int size; int cursorX; int cursorY; boolean setColor; char textString[])
  writeTextToOLED(2,0,16,0,"Yonaguska");
  delay(2000);
  display.clearDisplay();
  writeTextToOLED(1,0,16,1,myIP);
  delay(5000);
  writeTextToOLED(1,0,16,0,myIP);
  
  // start up the UDP port for SNTP
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
}

void getTimeFromNist() {
  time_t     now;
  TimeElements tm;
  int localTime;
  
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet"); 
    strncpy(timeDateAndTimeString, "Queried NIST NTP server, no packet yet", 40);
  } else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

     // Set the system time to the given time t
    now = secsSince1900 - seventyYears;
    setTime(now);
    makeTime(tm);
    localTime = hour(now) - 5; //Central time
    if (localTime < 0) {
      localTime +=24;
    }
    sprintf(timeDateAndTimeString, "%04d-%02d-%02d %02d:%02d:%02d", year(now), month(now), day(now), localTime, minute(now), second(now));
    sprintf(dateString, "%04d-%02d-%02d", year(now), month(now), day(now));
    sprintf(timeString, "%02d:%02d:%02d", localTime, minute(now), second(now));
    

    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.println(epoch % 60); // print the second

    // build our timestamp string
    int hour   = ((epoch  % 86400L) / 3600) -5; // for Central time
    int minute = (epoch % 3600) / 60;
    int second = epoch % 60;
    //sprintf(timeDateAndTimeString,"%02d:%02d:%02d", hour, minute, second);

  }

}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


void getWeather() {
    float sp;
    
    // get data from BME
    h = bme.readHumidity();
    t = bme.readTemperature();
    t = t*1.8+32.0;
    dp = t-0.36*(100.0-h);
    
    p = bme.readPressure()/100.0F; // pressure in Pascals, 100 Pascals is 1 hPa (aka millibar)
    pin = 0.02953*p;                                     // pressure in inches of Hg
    sp = bme.seaLevelForAltitude(myAltitudeInMeters, p); // sea level pressure for my altitude
    Serial.print("Sea level pressure for 196 meters ");  // Nevada, TX
    Serial.print(sp);
    a = bme.readAltitude(mySeaLevelPressure);
    af = a * 3.28;
    dtostrf(t, 5, 1, temperatureFString);
    dtostrf(h, 5, 1, humidityString);
    dtostrf(p, 6, 1, pressureString);
    dtostrf(pin, 5, 2, pressureInchString);
    dtostrf(dp, 5, 1, dpString);
    dtostrf(a, 5, 1, altitudeMetersString);
    dtostrf(af, 5, 1, altitudeFeetString);
    delay(100);

    // add a pressure sample to the array and check if we've filled the arrray. The pressureArrayIndex was initialized to 0
    pressureArray[pressureArrayIndex++] = p;
    if (pressureArrayIndex >= 100) {  //reset the index if we reached the end
      pressureArrayIndex = 0;
    }

    pressureTotal = 0.0;
    pressureSamples = 0;
    for (int i=0; i < pressureArrayMax; i++) {  //accumulate the total
      if ((pressureArray[i] > 500.0) && (pressureArray[i] < 2000.0)) {  // ... if it's sensible
        pressureTotal += pressureArray[i];
        pressureSamples++;
      }
    }
    // get our average
    pressureAverage = pressureTotal/pressureSamples;
    Serial.print("\nTotal pressure ");
    Serial.println(pressureTotal);
    Serial.print("number of samples ");
    Serial.println(pressureSamples);
    Serial.print("Pressure average ");
    Serial.println(pressureAverage);
    Serial.print("Pressure reading ");
    Serial.println(p);
    
    if (p < pressureAverage) {
      sprintf(pressureTrend, "%s ", "falling");
    } else if (p == pressureAverage) {
      sprintf(pressureTrend, "%s ", "level");
    } else  {
      sprintf(pressureTrend, "%s ", "rising");
    }
    pressureDifference = p - pressureAverage;
    dtostrf(pressureDifference, 5, 1, pressureDifferenceString);
}

void getSeaLevelPressureFromOpenWeatherMap() {
  //fiveMinutes = 60L;  // override to test logic ... 60 seconds
  unsigned long elapsedTimeInSeconds = secsSince1900 - lastWeatherPoll;

  if (elapsedTimeInSeconds >= fiveMinutes) {
    Serial.println("Here is where we call OpenWeatherMap for the current sea level pressure <<<<<<<<<<<<<<<<<<<");
    Serial.println("...poll for Lavon, TX: 4705584");
    //=======================================================================
    //starts client connection, checks for connection
    if (client.connect(servername, 80)) {
      client.println("GET /data/2.5/weather?id="+CityID+"&units=metric&APPID="+APIKEY);
      client.println("Host: api.openweathermap.org");
      client.println("User-Agent: ArduinoWiFi/1.1");
      client.println("Connection: close");
      client.println();
    }  else {
      Serial.println("connection failed"); //error message if no client connect
      Serial.println();
    }

    // wait for the data to come in
    while(client.connected() && !client.available()) delay(1); //waits for data
    while (client.connected() || client.available()) { //connected or data available
      char c = client.read(); //gets byte from ethernet buffer
        result = result+c;
    }
    
    // we have our response, stop the client and sed the buffer
    client.stop(); //stop client
    result.replace('[', ' ');
    result.replace(']', ' ');
    Serial.println(result);

    char jsonArray [result.length()+1];
    result.toCharArray(jsonArray,sizeof(jsonArray));
    jsonArray[result.length() + 1] = '\0';

    StaticJsonBuffer<1024> json_buf;
    //JsonObject &root = json_buf.parseObject(jsonArray);
    //if (!root.success()) {
    //  Serial.println("parseObject() failed");
    //}
    //=======================================================================
    mySeaLevelPressure = 1019.8;     // set with current reading, if exists
    lastWeatherPoll = secsSince1900; // update last time we got reading
    // get new data
    //JsonObject& root = jsonBuffer.parseObject(json);
    JsonObject &root = json_buf.parseObject(jsonArray);
    if(!root.success()) {
      Serial.println("JSON parseObject() failed");
    } else {
      Serial.println("JSON parseObject() succeeded");
      // get the important stuff
      cityName = root["name"];                        // "Lavon"
      Serial.print("City name:"); Serial.println(cityName);
      //if (cityName
      JsonObject& main = root["main"];
      mySeaLevelPressure = main["pressure"];          // 1017
      mySeaLevelPressure +=  1.5;                     // plus fudge factor
      // and the rest of the stuff
      coord_lon = root["coord"]["lon"];               // -96.43
      coord_lat = root["coord"]["lat"];               // 33.03
      Serial.print("||| Coordinates lontitude:"); Serial.println(coord_lon);
      Serial.print("||| Coordinates latitude:"); Serial.println(coord_lat);
      base = root["base"];                            // "stations"
      visibility = root["visibility"];                // 16093
      clouds_all = root["clouds"]["all"];             // 1
      dt = root["dt"];                                // 1504547700
      JsonObject& weather0 = root["weather"][0];
      weather0_id = weather0["id"];                   // 800
      weather0_main = weather0["main"];               // "Clear"
      weather0_description = weather0["description"]; // "clear sky"
      weather0_icon = weather0["icon"];               // "01d"
      main_temp = main["temp"];                       // 305.55
      main_pressure = main["pressure"];               // 1017
      main_humidity = main["humidity"];               // 49
      main_temp_min = main["temp_min"];               // 305.15
      main_temp_max = main["temp_max"];               // 306.15
      JsonObject& wind = root["wind"];
      wind_speed = wind["speed"];                     // 5.1
      wind_deg = wind["deg"];                         // 190
      wind_gust = wind["gust"];                       // 9.3
      Serial.print("||| Wind speed:"); Serial.println(wind_speed);
      Serial.print("||| Wind deg:"); Serial.println(wind_deg);
      Serial.print("||| Wind gust:"); Serial.println(wind_gust);
      if (wind_deg == 360) {
        wind_deg = 0;
      }
      wind_deg = 360 - wind_deg;
      where = deg2dir(wind_deg);
      Serial.print("||| Wind from:"); Serial.println(where);
      JsonObject& sys = root["sys"];
      sys_type = sys["type"];                         // 1
      sys_id = sys["id"];                             // 2678
      sys_message = sys["message"];                   // 0.0039
      sys_country = sys["country"];                   // "US"
      sys_sunrise = sys["sunrise"];                   // 1504526570
      sys_sunset = sys["sunset"];                     // 1504572320
    }
  } else {
    Serial.println("skip reading the current sea level pressure <<<<<<<<<<<<<<<<<<<");
    Serial.print("elapsedTimeInSeconds: ");
    Serial.println(elapsedTimeInSeconds);
    Serial.print(">>> City name:"); Serial.println(cityName);
    Serial.print(">>> Coordinates lontitude:"); Serial.println(coord_lon);
    Serial.print(">>> Coordinates latitude:"); Serial.println(coord_lat);
    Serial.print(">>> Wind speed:"); Serial.println(wind_speed);
    Serial.print(">>> Wind deg:"); Serial.println(wind_deg);
    Serial.print(">>> Wind gust:"); Serial.println(wind_gust);
    Serial.print(">>> Wind from:"); Serial.println(where);
  }
  Serial.print("mySeaLevelPressure: ");
  Serial.println(mySeaLevelPressure);
  
}

// runs over and over again
void loop() {
  // get our current time from NIST
  getTimeFromNist();
  // get our Sea Level Pressure from OpenWeatherMap
  getSeaLevelPressureFromOpenWeatherMap();
  // read our BME
  getWeather();


  // Write to our OLED
  display.clearDisplay();
  writeTextToOLED(1,0,8,1,timeDateAndTimeString);
  sprintf(myOLEDString,"Temp        %s \%F", temperatureFString);
  writeTextToOLED(1,0,17,1,myOLEDString);
  sprintf(myOLEDString,"Humidity    %s \%", humidityString);
  writeTextToOLED(1,0,26,1,myOLEDString);
  sprintf(myOLEDString,"Dew Point   %s \%F", dpString);
  writeTextToOLED(1,0,35,1,myOLEDString);
  sprintf(myOLEDString,"Barometer   %s In", pressureInchString);
  writeTextToOLED(1,0,44,1,myOLEDString);
  sprintf(myOLEDString,"... %s %s mB", pressureTrend, pressureDifferenceString);
  writeTextToOLED(1,0,53,1,myOLEDString);
    
  // Listenning for new clients
  //Serial.println("Listening for new clients");
  WiFiClient client = server.available();
  
  if (client) {
    Serial.println("New client");
    // bolean to locate when the http request ends
    boolean blank_line = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && blank_line) {
            Serial.print("### City name:"); Serial.println(cityName);
            Serial.print("### Coordinates lontitude:"); Serial.println(coord_lon);
            Serial.print("### Coordinates latitude:"); Serial.println(coord_lat);
            Serial.print("### Wind speed:"); Serial.println(wind_speed);
            Serial.print("### Wind deg:"); Serial.println(wind_deg);
            Serial.print("### Wind gust:"); Serial.println(wind_gust);
            Serial.print("### Wind from:"); Serial.println(where);
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            // your actual web page that displays temperature
            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head><META HTTP-EQUIV=\"refresh\" CONTENT=\"15\"></head>");
            if (debug) {
              client.println("<body><h1>");
              client.println(nodeType);
              client.println(" Weather Web Server</h1>");
            } else {
              client.println("<body><h1>Weather Web Server - no debug</h1>");
            }
            client.println("<table border=\"2\" width=\"456\" cellpadding=\"10\"><tbody><tr><td>");
            client.println("<h3>");
            client.println(timeDateAndTimeString);
            client.println("</h3>");
            client.println("<h3>Temperature is ");
            client.println(temperatureFString);
            client.println("&deg;F</h3>");
            client.println("<h3>Humidity is ");
            client.println(humidityString);
            client.println("%</h3>");
            client.println("<h3>Approx. Dew Point is ");
            client.println(dpString);
            client.println("&deg;F</h3>");
            client.println("<h3>Barometer is ");
            client.println(pressureString);
            client.println("mB (");
            client.println(pressureInchString);
            client.println("Inch) ");
            client.println(pressureTrend);
            client.println(pressureDifference);
            client.println(" mB");
            client.println("</h3>");
            if (weather0_main = NULL) {              
              client.println("<h3>Sky is ");
              client.println(weather0_main);
              client.println("</h3>");
            }
            client.println("<h3>Wind at ");
            client.println(wind_speed * 2.2369362920544);
            client.println(" MPH");
            if (wind_gust > 0.0) {
              client.println(", gusting to ");
              client.println(wind_gust * 2.2369362920544);
              client.println(" MPH</h3>");
            }
            client.println("<h3>Wind from ");
            client.println(wind_deg);
            client.println(" degrees</h3>");
            client.println("<h3>Wind ");
            client.println(where);
            client.println("</h3>");
            
            if (debug) {
              client.println("<h3>Altitude is ");
              client.println(altitudeFeetString);
              client.println(" feet");
              if (validString(cityName)) {
                 client.println(" in ");
                 client.println(cityName);
              }
              client.println("</h3>");
              client.println("<h3>Latitude: ");
              client.println(coord_lat);
              client.println(" Longtitude: ");
              client.println(coord_lon);
              client.println("</h3>");
            }
            client.println("</td></tr></tbody></table></body></html>");   
            break;
        }
        if (c == '\n') {
          // when starts reading a new line
          blank_line = true;
        }
        else if (c != '\r') {
          // when finds a character on the current line
          blank_line = false;
        }
      }
    }  
    // closing the client connection
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
  delay(5000);
  writeTextToOLED(1,0,8,0,timeDateAndTimeString);
  sprintf(myOLEDString,"Temp        %s \%F", temperatureFString);
  writeTextToOLED(1,0,17,0,myOLEDString);
  sprintf(myOLEDString,"Humidity    %s \%", humidityString);
  writeTextToOLED(1,0,26,0,myOLEDString);
  sprintf(myOLEDString,"Dew Point   %s \%F", dpString);
  writeTextToOLED(1,0,35,0,myOLEDString);
  sprintf(myOLEDString,"Barometer   %s In", pressureInchString);
  writeTextToOLED(1,0,44,0,myOLEDString);
  sprintf(myOLEDString,"... %s %s mB", pressureTrend, pressureDifferenceString);
  writeTextToOLED(1,0,53,0,myOLEDString);

} 
