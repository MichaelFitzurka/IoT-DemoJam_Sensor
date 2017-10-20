/**
 * DHTServer - ESP8266 with a DHT sensor as an input
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <PubSubClient.h>

extern "C" {
  #include "user_interface.h"
  uint16 readvdd33(void);
}

/************************* DHT22 Sensor *************************/
#define DHTPIN  D2    // what pin we're connected to
#define DHTTYPE DHT22 // DHT 22  (AM2302)

/************************* WiFi Access Point *************************/
const char* ssid     = "IoT-DemoJam_Wifi"; // Router Name
const char* password = "DLT-iot-123!";  // Router Password

/************************* MQTT Server *************************/
char*       mqtt_server      = "192.168.106.200"; // IP of Raspberry Pi SmartGateway
int         mqtt_server_port = 1883;              // Port of Fuse on Pi
const char* mqtt_user        = "admin";           // Username on Fuse on Pi
const char* mqtt_password    = "DLT-iot-123!";    // Password on Fuse on Pi
String      message          = "";
String      topicTemp        = "";
String      topicHumid       = "";
String      topicVoltage     = "";

/************************* Measurements *************************/
float         humidity, temp, maxTemp = 0.0; // Values read from sensor
int           voltage;                       // Sensor voltage
String        chipId          = "";          // ESP chip id
int           lightState      = HIGH;        // State of light (LOW=On,HIGH=Off)
char*         payloadChipId   = "";

unsigned long previousMillis = 0;            // will store last temp was read
const long    interval       = 2000;         // interval at which to read sensor

unsigned long count          = 0;            // counter for message points



/************************* ESP8266 WiFiClient/WebServer *************************/
WiFiClient wifiClient;
ESP8266WebServer server(80);

/************************* DHT Sensor *************************/
DHT dht(DHTPIN, DHTTYPE);

/************************* Prototypes *************************/
void callback(char* topic, byte* payload, unsigned int length);

/************************* MQTT Client *************************/
PubSubClient client(mqtt_server, mqtt_server_port, callback, wifiClient);



/************************* Light Controls *************************/
void setLightState(int state) {
  digitalWrite(BUILTIN_LED, state);
  lightState = state;
}

void turnLightOff() {
  setLightState(HIGH);
}

void turnLightOn() {
  setLightState(LOW);
}

void blinkLight(int blinkCount) {
  int lightStateHold = lightState;
  for (unsigned i = 1; i <= blinkCount; ++i) {
    turnLightOn();
    delay(500);
    turnLightOff();
    delay(500);
  }
  setLightState(lightStateHold);
}

void controlLight(char* state) {
  if (strstr(state, "on") != NULL) {
    Serial.println("Setting light to: on");
    turnLightOn();
  } else if (strstr(state, "off") != NULL) {
    Serial.println("Setting light to: off");
    turnLightOff();
  } else if (strstr(state, "blink") != NULL) {
    Serial.println("Setting light to: blink");
    blinkLight(5);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  if (strstr(((char*) payload), payloadChipId) != NULL) {
    controlLight((char*) payload);
  }
}

/************************* Utility function to retrieve data from DHT22 *************************/
void getTemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor
    previousMillis = currentMillis;

    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();        // Read humidity (percent)
    temp = dht.readTemperature();         // Read temperature as Celsius
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp) || humidity == 2147483647 || temp == 2147483647) { // 2147483647 = MAX_INT (which happens when circuit is loose.)
      Serial.println("Failed to read from DHT sensor!");
      return;
    } else if (temp > maxTemp) {
      Serial.println("New max temp: " + (int) temp);
      maxTemp = temp;
    }
  }
}

/************************* WebServer Methods *************************/
void handleRoot() {
  String response = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"application-name\" content=\"iotdj-sensor\"><meta name=\"author\" content=\"Michael Fitzurka @ DLT Solutiuons, Inc.\"><meta name=\"description\" content=\"IoT DemoJam :: Sensor\"><title>IoT DemoJam :: Sensor :: Status</title></head><body><header><h1>ESP8266 #";
  response += chipId;
  response += " - Status Report</h1></header><section><h3>Temperature: ";
  response += (int) temp;
  response += " &#8451 (";
  response += (int) dht.convertCtoF(temp);
  response += " &#8457) <a href=\"/maxTemp\">Max Temp</a>: ";
  response += (int) maxTemp;
  response += " &#8451 (";
  response += (int) dht.convertCtoF(maxTemp);
  response += " &#8457)</h3><h3>Humidity: ";
  response += (int) humidity;
  response += " %</h3><h3>Heat Index: ";
  response += (int) dht.computeHeatIndex(temp, humidity, false);
  response += "</h3><h3>Voltage: ";
  response += voltage;
  response += " mV</h3><h3>At Count: ";
  response += count;
  response += "</h3><h3>WiFi Connection to: ";
  response += ssid;
  response += "</h3><h3>SmartGateway Located at: ";
  response += mqtt_server;
  response += ":";
  response += mqtt_server_port;
  response += "</h3><h3><a href=\"/light\">Light</a> is: ";
  response += (lightState == LOW) ? "On" : "Off";
  response += "</h3><h3><a href=\"/\">Reload</a></h3></section></body></html>";
  server.send(200, "text/html", response);
}

void handleLight() {
  String state = server.arg("state");

  controlLight((char*) state.c_str());

  String response = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"application-name\" content=\"iotdj-sensor\"><meta name=\"author\" content=\"Michael Fitzurka @ DLT Solutiuons, Inc.\"><meta name=\"description\" content=\"IoT DemoJam :: Sensor\"><title>IoT DemoJam :: Sensor :: Light</title></head><body><header><h1>ESP8266 #";
  response += chipId;
  response += " - Light Control</h1></header><section><h3><a href=\"/light\">Light</a> is: ";
  response += (lightState == LOW) ? "On" : "Off";
  response += "</h3><h3>Set Light to: ";
  response += (lightState == LOW) ? "On " : "<a href=\"/light?state=on\">On</a> ";
  response += (lightState == HIGH) ? "Off " : "<a href=\"/light?state=off\">Off</a> ";
  response += "<a href=\"/light?state=blink\">Blink</a> (5 times)</h3><h3><a href=\"/\">Status</a></h3></section></body></html>";
  server.send(200, "text/html", response);
}

void handleMaxTemp() {
  String response = "";
  response += (int) maxTemp;
  server.send(200, "text/plain", response);
}

void handleNotFound() {
  String response = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"application-name\" content=\"iotdj-sensor\"><meta name=\"author\" content=\"Michael Fitzurka @ DLT Solutiuons, Inc.\"><meta name=\"description\" content=\"IoT DemoJam :: Sensor\"><title>IoT DemoJam :: Sensor :: 404</title></head><body><header><h1>ESP8266 #";
  response += chipId;
  response += " - 404</h1></header><section><h3>File Not Found</h3><h3>URI: ";
  response += server.uri();
  response += "</h3><h3>Method: ";
  response += (server.method() == HTTP_GET) ? "GET" : "POST";
  response += "</h3><h3>Arguments: ";
  response += server.args();
  response += "</h3><ul>";
  for (uint8_t i = 0; i < server.args(); i++) {
    response += "<li>" + server.argName(i) + ": " + server.arg(i) + "</li>";
  }
  response += "</ul></section></body></html>";
  server.send(404, "text/html", response);
}

/************************* Function name says it all! *************************/
void setup(void){
  Serial.begin(115200);

  pinMode(BUILTIN_LED, OUTPUT);

  turnLightOff();

  dht.begin();

  // Display chip id
  chipId = String(ESP.getChipId());
  Serial.print("\n\rChip-ID: ");
  Serial.println(chipId);

  // Create strings for MQTT topics
  topicTemp    = "iotdj/Temperature/" + chipId;
  topicHumid   = "iotdj/Humidity/" + chipId;
  topicVoltage = "iotdj/Voltage/" + chipId;

  payloadChipId = strdup((chipId + "|").c_str());

  // Connect to WiFi network
  WiFi.begin(ssid, password);
  Serial.print("\n\rConnecting to: ");
  Serial.println(ssid);

  // Start the web server
  server.on("/", handleRoot);
  server.on("/light", handleLight);
  server.on("/maxTemp", handleMaxTemp);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("\n\rWeb Server started.");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n\rESP8266 & DHT22 based temperature and humidity sensor working!");

  // Display IP address
  Serial.print("\n\rIP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

/************************* Utility function to connect or re-connect to MQTT-Server *************************/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection to: ");
    Serial.print(mqtt_server);
    Serial.print(" with: ");
    Serial.print(mqtt_user);
    Serial.print(" / ");
    Serial.print(mqtt_password);

    // Attempt to connect
    if (client.connect(String(ESP.getChipId()).c_str(), mqtt_user, mqtt_password)) {
      Serial.println("Connected!");

      // subscribe to topic
      if (client.subscribe("iotdj-command/light")){
        Serial.println("Successfully subscribed!");
      }

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds.");

      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/************************* Function name says it all! *************************/
void loop(void){
    // Connect to MQTT broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read data
  getTemperature();
  voltage = readvdd33();

  // Publish temperature
  message = String((int) temp) + ", " + String(count);
  Serial.print("Sending temperature value in Celsius <");
  Serial.print(message);
  Serial.println(">");
  if (isnan(temp) || temp == 2147483647) { // 2147483647 = MAX_INT (which happens when circuit is loose.)
    Serial.println("Invalid temperature, NOT publishing.");
  } else {
    client.publish(topicTemp.c_str(), message.c_str());
  }

  // Publish humidity
  message = String((int) humidity) + ", " + String(count);
  Serial.print("Sending humidity value <");
  Serial.print(message);
  Serial.println(">");
  if (isnan(humidity) || humidity == 2147483647) { // 2147483647 = MAX_INT (which happens when circuit is loose.)
    Serial.println("Invalid humidity, NOT publishing.");
  } else {
    client.publish(topicHumid.c_str(), message.c_str()); // Comment out this line to reduce traffic for demo, if desired.
  }

  // Publish voltage
  message = String((int) voltage) + ", " + String(count);
  Serial.print("Sending sensor voltage <");
  Serial.print(message);
  Serial.println(">");
  client.publish(topicVoltage.c_str(), message.c_str()); // Comment out this line to reduce traffic for demo, if desired.

  // Increase counter
  count = count + 1;

  // Handle any web server request
  server.handleClient();

  // Pause, rinse & repeat
  delay(1000);
}

