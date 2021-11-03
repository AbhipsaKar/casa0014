/*
 *   File name: DHT22_MQTT.ino
 *   Desc: Get date and time - uses the ezTime library at https://github.com/ropg/ezTime -
 *   and then show data from a DHT22 on a web page served by the Huzzah and
 *   push data to an MQTT server - uses library from https://pubsubclient.knolleary.net
 *
 *   Author: Duncan Wilson
 *   CASA0014 - 2 - Plant Monitor Workshop
 *   May 2020
 *
 *   Modified by: Abhipsa Kar
 *   CASA0014 - Plant Monitor workshop
 *   Nov 2021
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ezTime.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Sensors - DHT22 and Nails
uint8_t DHTPin = 12;        // D12 pin of MCu to be connected to the DHT sensor.
uint8_t soilPin = 0;      // ADC or A0 pin on Huzzah connected to soil sensor
float Temperature;
float Humidity;
int Moisture = 0; // initial value just in case web page is loaded before readMoisture called
int sensorVCC = 13;  // D13 pin of MCu to be connected to the transistor.
int blueLED = 2;
DHT dht(DHTPin, DHTTYPE);   // Initialize DHT sensor.
int cold= 0;
int hot= 0;
int wet = 0;
int dry = 0;

// Wifi and MQTT
#include "arduino_secrets.h" 

const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;

ESP8266WebServer server(80);
const char* mqtt_server = "mqtt.cetools.org";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// 60000 = 1 minute in milliseconds
long interval = 60000; // interval at which to take the next sensor reading
long previousTime = 0; //Initialise previous time

// Date and time
Timezone GB;

void setup() {
  // Set up LED to be controllable via broker
  // Initialize the BUILTIN_LED pin as an output
  // Turn the LED off by making the voltage HIGH
  pinMode(BUILTIN_LED, OUTPUT);     
  digitalWrite(BUILTIN_LED, HIGH);  

  // Set up the outputs to control the soil sensor
  // switch and the blue LED for status indicator
  pinMode(sensorVCC, OUTPUT); 
  digitalWrite(sensorVCC, LOW);
  pinMode(blueLED, OUTPUT); 
  digitalWrite(blueLED, HIGH);  //Initialise the blue LED to OFF

  // open serial connection for debug info
  Serial.begin(115200);
  delay(100);

  // start DHT sensor
  pinMode(DHTPin, INPUT);
  dht.begin(); //Start the temp,humidity sensor

  // run initialisation functions
  startWifi();
  startWebserver();
  syncDate(); //Get the current date from ezTime server.

  // start MQTT server
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback); //register callback function for MQTT server

}

void loop() {
  // handler for receiving requests to webserver
  server.handleClient();

  //When the minute has changed,take the sensor readings.
    // store the time since the Arduino started running in a variable
  unsigned long currentTime = millis();

  // compare the current time to the previous time an LED turned on
  // if it is greater than your interval, run the if statement
  if (currentTime - previousTime > interval) {
 // if (minuteChanged()) {
    previousTime = currentTime;
    readMoisture();
    sendMQTT();
    Serial.println(GB.dateTime("H:i:s")); // UTC.dateTime("l, d-M-y H:i:s.v T")
  }
  
  client.loop();
}

/* Function: readMoisture()
 * Desc: Function is called every time the 
 * sensors are switched on to take the moisture
 * readings.
 */
void readMoisture(){
  
  
  digitalWrite(sensorVCC, HIGH); // connect power to the sensor network
  digitalWrite(blueLED, LOW); //Set the blue LED to ON while taking sensor readings
  delay(100);
  // read the value from the sensor:
  Moisture = analogRead(soilPin);         
  
  //stop power 
  digitalWrite(sensorVCC, LOW);  // disconnect the sensor network
  digitalWrite(blueLED, HIGH); //Set the blue LED to ON while taking sensor readings
  delay(100);
  Serial.print("Moisture reading: ");
  Serial.println(Moisture);   // read the value from the soil moisture sensor
}

/* Function: startWifi()
 * Desc: Use the ESP wifi libraries
 * to create the WIFI network with
 * the provided credentials.
 */
void startWifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // check to see if connected and wait until you are
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void syncDate() {
  // get real date and time
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
  GB.setLocation("Europe/London");
  Serial.println("London time: " + GB.dateTime());

}
void startWebserver() {
  // when connected and IP address obtained start HTTP server
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}
/* Function: sendMQTT
 * Desc: Function to fetch the humidity/temperature reading
 * and then publish all the sensor readings to MQTT server 
 * on topic: student/CASA0014/plant/ucfnaka/
 * Also to subscribe to inTopic. 
 * This function will get called every minute from loop().
 */
void sendMQTT() {

  if (!client.connected()) {
    reconnect();  /* Reconnect if not connected. Trigger: First iteration and server disconnection*/
  }
  client.loop();

  Temperature = dht.readTemperature(); // Gets the values of the temperature
  snprintf (msg, 50, "%.1f", Temperature);
  Serial.print("Publish message for t: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnaka/temperature", msg);

  Humidity = dht.readHumidity(); // Gets the values of the humidity
  snprintf (msg, 50, "%.0f", Humidity);
  Serial.print("Publish message for h: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnaka/humidity", msg);

  //Moisture is read within its own function
  
  //Publish Moisture reading
  snprintf (msg, 50, "%.0i", Moisture);
  Serial.print("Publish message for m: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnaka/moisture", msg);

}

/* Function: callback()
 * Desc: Function is called each time sensor
 * readings are published on MQTT server which you have subscribed to:
 * on topic: student/CASA0014/plant/ucfnaka/inTopic
 */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]); //Print each char of the message published on MQTT server.
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

/* Function: reconnect()
 * Desc: Function is called when sendMqtt()
 * is called for the first time. If server 
 * gets disconnected, keep retrying to establish
 * server connection before publishing data.
 */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-Abhipsa";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect with clientID, username and password
    if (client.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("student/CASA0014/plant/ucfnaka/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
/* Function: handle_OnConnect()
 * Desc: send sensor data to HTTP server
 */
void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  plantTempSuggest(Temperature);
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  plantHumSuggest(Humidity);
  server.send(200, "text/html", SendHTML(Temperature, Humidity, Moisture));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

/* Function: plantTempSuggest()
 * Desc: Determine if the ambient temp
 * is too hot or too cold for the plant
 */
void plantTempSuggest(float Temperaturestat){
  if (Temperaturestat <15 )
    cold =1;
   else 
    cold = 0;
  Serial.print("Cold =" + cold);
  if (Temperaturestat >21 )
    hot = 1;
   else 
    hot =0; 
  Serial.print("Hot =" + hot);
}

/* Function: plantHumSuggest()
 * Desc: Determine if the soil
 * is too wet or too dry for the plant
 */
void plantHumSuggest(float Moisturestat){
  if (Moisturestat >50 )
    wet =1;
   else 
    wet =0;
  if (Moisturestat <6 )
    dry = 1;
   else 
    dry =0; 
}

/* Function: SendHTML()
 * Desc: callback function to create the webpage 
 * to display the sensor readings as well as suggestions.
 */
String SendHTML(float Temperaturestat, float Humiditystat, int Moisturestat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP8266 DHT22 Report</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP8266 Huzzah DHT22 Report-Christmas cactus</h1>\n";

  ptr += "<p>Temperature: ";
  ptr += (int)Temperaturestat;
  ptr += " C</p>";
  ptr += "<p>Humidity: ";
  ptr += (int)Humiditystat;
  ptr += "%</p>";
  ptr += "<p>Moisture: ";
  ptr += Moisturestat;
  ptr += "</p>";
  ptr += "<p>Sampled on: ";
  ptr += GB.dateTime("l,");
  ptr += "<br>";
  ptr += GB.dateTime("d-M-y H:i:s T");
  ptr += "</p>";
  ptr += "<p><h1> Plant suggestions </h1></p>";
  ptr += "<p>Am I too cold?  ";
  ptr += (int)cold?"YES":"NO";
  ptr += "</p>";
  ptr += "<p>Am I too warm?  ";
  ptr += (int)hot?"YES":"NO";
  ptr += "</p>";
  ptr += "<p>Am I too wet?  ";
  ptr += (int)wet?"YES":"NO";
  ptr += "</p>";
  ptr += "<p>Am I too dry?  ";
  ptr += (int)dry?"YES":"NO";
  ptr += "</p>";
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
