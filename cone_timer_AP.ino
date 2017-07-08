#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <VL53L0X.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__  
#include <avr/power.h>
#endif
#define PIN            14   // Which pin on the Arduino is connected to the NeoPixels?
#define NUMPIXELS      16   // How many NeoPixels are attached to the Arduino?

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);   // When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.

VL53L0X sensor;   //initialize sensor

//sets username and password for the access point
const char *ssid = "Cone1";
const char *password = "chasr123";

ESP8266WebServer server(80);    //starts the server on port 80

/* Go to http://192.168.4.1 in a web browser */

String webPage = "";    //things printed on the webpage
String str;   //the string holds the data detected

// set pin numbers:
const int buttonPin = 12;     // the number of the pushbutton pin
// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void handleRoot() {
  server.send(200, "text/html", webPage);   //sends the string webPage in HTML format through HTTP
}

// the setup routine runs once when you press reset:
void setup() {
  
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

  //access point setting
  // You can remove the password parameter if you want the AP to be open.
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  //Arduino code format for HTML webpage
  webPage += "<HTML>";
  webPage += "<HEAD>";
  webPage += "<TITLE>SENSOR</TITLE>";
  webPage += "</HEAD>";
  webPage += "<BODY style=\"zoom:3\">";
  webPage += "<font size=\"5\"> Start Timer </font>";
  webPage += "<br/>";
  webPage += "<a href=\"ON\"><button style=\"display: inline-block\"><font size=\"20\"> Start </font></button></a>";
  webPage += "<br/>";
  webPage += "<font size=\"5\"> Time Detected(seconds): </font>";
  webPage += "<br/>";
  webPage += "First | Second |";
  webPage += "<br/>";
  webPage += "<BODY/>";
  webPage += "</HTML>";

  //sensor setting  
  Wire.begin();  
  Wire.pins(4, 5);    
  sensor.init();  
  sensor.setTimeout(500); // Start continuous back-to-back mode (take readings as fast as possible).  To use continuous timed mode instead, provide a desired inter-measurement period in  ms (e.g. sensor.startContinuous(100)).  
  sensor.startContinuous();  

  //begin neopixel
  pixels.begin();

  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);

  //tell the server what URI it needs to respond to
  server.on("/", handleRoot);

  //tell the server what URI it needs to respond to
  server.on("/", [](){
     server.send(200, "text/html", webPage);
  });

  //run the code below after the button on the webpage is clicked
  //clicking on button modifies the URL
  server.on("/ON", [](){
      //keeps detecting the button state untill it is pressed
      while(buttonState == 0){
        buttonState = digitalRead(buttonPin);  
        delay(1);
      }
      
      int pass = 0;   //number of athletes passing by the sensor
      double t;   //the final time shown on the webpage
      double t1 = millis();   //records the current time into variable t1 when the button is clieked
      while(pass<2){
        //if it is the first pass
        if(pass=0){
          delay(1000);
        }
        //if it is the second pass
        else{
          delay(500);
        }
        
        int range = 9999;   //the distance between the athlete and the sensor
                            //range is set to 9999 to meet the initial condition of the while loop next line
                            
        while (range>5000 || range<10){
          range = sensor.readRangeContinuousMillimeters();    //detect the distance
          delay(0);   //to ensure processing withour the ESP8266 crashing
        }
        double t2 = millis();   //records the current time into variable t2 when the athlete passes by the cone
        t = (t2-t1)/1000.00;    //calculate the difference of the time recorded at t1 and t2 in seconds
        str +=  String(t, 2);   //convert the time into a string
        str += " | ";   //included to make UI better
        
        Serial.println(t1);
        Serial.println(t2);
        Serial.println(str);

        //Neopixel animation
        for(int i=0; i<16; i++){    
           pixels.setPixelColor(i, pixels.Color(0,40,0));    
           pixels.show();
           delay(20); 
        }

        pass++;   //value of pass increases by 1 everytime it goes through the while loop

        //turn off Neopicel animation
        for(int i=0; i<16; i++){    
          pixels.setPixelColor(i, pixels.Color(0,0,0));    
          pixels.show();
        }
      }

      //error checking of the sensor
      if (sensor.timeoutOccurred()) { 
        Serial.print(" TIMEOUT"); 
      }

      //upload data(stored in string str) to the webpage
      webPage += "<HTML>";
      webPage += "<HEAD>";
      webPage += "<TITLE>SENSOR</TITLE>";
      webPage += "</HEAD>";
      webPage += "<BODY>";
      webPage += str;
      str = "";   //clears str after uploading
      webPage += "<br/>";
      webPage += "<BODY/>";
      webPage += "</HTML>";
      server.send(200, "text/html", webPage);   //updates the webpage in the format above
  });

  //starts the HTTP server
  server.begin();
  Serial.println("HTTP server started");
}

// the loop routine runs over and over again forever:
void loop() {
  server.handleClient();    //funtion to keep the server running online
}
