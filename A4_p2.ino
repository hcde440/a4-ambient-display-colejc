//A4 part 2, grab JSON and display

#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_MQTT.h>
#include <Adafruit_MPL115A2.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>    //Requisite Libraries . . .
#include <ESP8266HTTPClient.h>
#include "Wire.h"           //
#include <PubSubClient.h>   //
#include <ArduinoJson.h>    //

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define wifi_ssid "University of Washington"   //You have seen this before
#define wifi_password "" //

#define mqtt_server "mediatedspaces.net"  //this is its address, unique to the server
#define mqtt_user "hcdeiot"               //this is its server login, unique to the server
#define mqtt_password "esp8266"           //this is it server password, unique to the server

WiFiClient espClient;             //blah blah blah, espClient
PubSubClient mqtt(espClient);     //blah blah blah, tie PubSub (mqtt) client to WiFi client

char mac[6]; //A MAC address is a 'truly' unique ID for each device, lets use that as our 'truly' unique user ID!!!
char message[201]; //201, as last character in the array is the NULL character, denoting the end of the array

/////SETUP_WIFI/////
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  //get the unique MAC address to use as MQTT client ID, a 'truly' unique ID.
  Serial.println(WiFi.macAddress());  //.macAddress returns a byte array 6 bytes representing the MAC address
}                                     //5C:CF:7F:F0:B0:C1 for example

/////SETUP/////
void setup() {
  Serial.begin(115200);
  setup_wifi();
  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback); //register the callback function
  
  //SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.display();
  delay(2000); // Pause for 2 seconds
  display.clearDisplay();
}

/////CONNECT/RECONNECT/////Monitor the connection to MQTT server, if down, reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(mac, mqtt_user, mqtt_password)) { //<<---using MAC as client ID, always unique!!!
      Serial.println("connected");
      mqtt.subscribe("fromJon/+"); //we are subscribing to 'theTopic' and all subtopics below that topic
    } else {                        //please change 'theTopic' to reflect your topic you are subscribing to
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////LOOP/////
void loop() {
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop(); //this keeps the mqtt connection 'active'
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println();
  Serial.println("Message arrived.");

  //Message only sent if sensor triggered, so we can safely go ahead and output our API message.

  //CALL API AND GET A MESSAGE
  HTTPClient theClient;
  //Serial.println("Making HTTP request");
  String apiCall = "http://api.kanye.rest/";

  //Collect the information from the URL, and check to make sure we haven't encountered any errors.
  theClient.begin(apiCall);
  int httpCode = theClient.GET();
  //Serial.println(httpCode);
  if (httpCode > 0) {
    if (httpCode == 200) {
      
      //Print a line that lets us know the correct payload has been received, and organize the information
      //we've grabbed from online in JSON format in the same way we did for getIP().
      //Serial.println("Received HTTP payload.");
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      //Serial.println("Parsing...");
      JsonObject& root = jsonBuffer.parse(payload);
      // Test if parsing succeeds, and if it didn't, output a notice.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }

      //Fill in the joke String in the joke instance
      String quote = root["quote"].as<String>();

      //PRINT TO LCD SCREEN
      display.clearDisplay();
      display.setCursor(0,0);
      display.print("Quote of the Day:");
      display.setCursor(1,0);
      display.print(quote);
      //display.print(quote(0,29));
      //display.setCursor(2,0);
      //display.print(quote(30,59));

      Serial.println("Kanye quote of the day:");
      Serial.println(quote);
      Serial.println();      
      Serial.println();
      Serial.println();

    //Output a line in case httpCode is the incorrect value.
    } else {
      Serial.println("Something went wrong with connecting to the endpoint.");
    } 
  }
}
