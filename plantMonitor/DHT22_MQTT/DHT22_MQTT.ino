/*
    Collects humidity, temperature and moisture readings from the DHT22 and nail-based sensors.
    Publishes readings to both a web server and an MQTT server.  
    
    August Weinbren
    CASA0014 - 2 - Plant Monitor Workshop
    Nov 2021
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ezTime.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

// Sensors - DHT22 and Nails
uint8_t DHTPin = 12;        // on Pin 2 of the Huzzah
uint8_t soilPin = 0;      // ADC or A0 pin on Huzzah
float Temperature;
float Humidity;
int Moisture = 1; // initial value just in case web page is loaded before readMoisture called
int sensorVCC = 13;
int blueLED = 2;
DHT dht(DHTPin, DHTTYPE);   // Initialize DHT sensor.


// Wifi and MQTT

/// \note "An arduino_secrets.h" file needs to be created in the DHT22_MQTT directory which defines ssid,
/// password, etc. in the following format:
/// #define SECRET_SSID "insert_ssid_here"
#include "arduino_secrets.h" 

const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;

// \note Please refer to the following two initialised variables for viewing the output data.
// As the ESP8266WebServer is initialised with a host port number of 80, the data can be accessed
// from a web browser by typing in: [ip address]:80
ESP8266WebServer server(80);
const char* mqtt_server = "mqtt.cetools.org";

WiFiClient espClient;
PubSubClient pubSubClient(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

// Date and time
Timezone timeZone;
const char* continentCity = "Europe/London";


///
/// 1. Set builtin LED as an output pin and initialised as off (set to high).
/// 2. Set sensorVCC and blueLED as output pins, which will control the sensor;
/// sensorVCC as low and blueLED as high will leave the sensor turned off.
/// 3. Serial connection opened to 115200 bits/s for debugging
/// 4. DHTPin marked as input and activated in anticipation of sensorVCC being turned on.
/// 5. WiFi started, webserver started, and date-time set
/// 6. MQTT server started.
///
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
  digitalWrite(blueLED, HIGH);

  // open serial connection for debug info (115200 bauds)
  Serial.begin(115200);
  delay(100);

  // start DHT sensor
  pinMode(DHTPin, INPUT);
  dht.begin();

  // run initialisation functions
  startWifi();
  startWebserver();
  syncDate();

  // start MQTT server on the 1884 port
  pubSubClient.setServer(mqtt_server, 1884);
  pubSubClient.setCallback(callback);

}

///
/// The loop:
/// 1. Handles clients.
/// 2. Upon changing of the minute:
///    a. moisture is read and the value is sent to the MQTT.
///    b. Timestamp is printed in the serial log for debugging.
/// 3. Messages processed by client
///

void loop() {
  // handler for receiving requests to webserver.
  // Specifically, listens for HTTP requests, calling fxns set with server.on().
  server.handleClient();

  if (minuteChanged()) {
    readMoisture();
    sendMQTT();
    Serial.println(timeZone.dateTime("H:i:s")); // UTC.dateTime("l, d-M-y H:i:s.v T")
  }

  // allows client to process incoming msgs and maintain connection to server
  pubSubClient.loop();
}

/// 
/// Reads moisture by powering on the sensor and waiting for the values to stabilise before taking an
/// analog reading. moisture values are then remapped from the empirically observed low and high values
/// to the full range of 0-100. Sensor is turned off again and the moisture value is printed.
///
void readMoisture(){
  
  // power the sensor
  digitalWrite(sensorVCC, HIGH);
  digitalWrite(blueLED, LOW);
  delay(100);
  // read the value from the sensor:
  Moisture = analogRead(soilPin);
  /// \note these values may need to change.         
  Moisture = map(Moisture, 5, 20, 0, 100);    // note: if mapping work out max value by dipping in water     
  //stop power
  digitalWrite(sensorVCC, LOW);  
  digitalWrite(blueLED, HIGH);
  delay(100);
  Serial.print("Wet ");
  Serial.println(Moisture);   // read the value from the nails
}

///
/// Connect to the pre-defined wifi network. Will continue attempting
/// to connect indefinitely. IP address (necessary for accessing data from
/// browser) will be printed.
///
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
  // This IP address can be used to access the associated website
  Serial.println(WiFi.localIP());
}


///
/// Get the date-time based on London time.
///
void syncDate() {
  // get real date and time
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
  timeZone.setLocation(continentCity);
  Serial.print(continentCity);
  Serial.println(" time: " + timeZone.dateTime());

}

///
/// Will begin server based on client requests (both valid and invalid)
///
void startWebserver() {
  // when connected and IP address obtained start HTTP server
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");
}

///
/// Collects readings of temperature and humidity.
/// Publishes readings of moisture, humidity and temperature but does not need to collect measurement
/// because moisture was measured prior to running sendMQTT().
///
void sendMQTT() {

  if (!pubSubClient.connected()) {
    reconnect();
  }
  // allows client to process all incoming messages
  pubSubClient.loop();

  Temperature = dht.readTemperature(); // Gets the values of the temperature
  snprintf (msg, 50, "%.1f", Temperature);
  Serial.print("Publish message for t: ");
  Serial.println(msg);
  pubSubClient.publish("student/CASA0014/plant/ucfnawe/temperature", msg);

  Humidity = dht.readHumidity(); // Gets the values of the humidity
  snprintf (msg, 50, "%.0f", Humidity);
  Serial.print("Publish message for h: ");
  Serial.println(msg);
  pubSubClient.publish("student/CASA0014/plant/ucfnawe/humidity", msg);

  snprintf (msg, 50, "%.0i", Moisture);
  Serial.print("Publish message for m: ");
  Serial.println(msg);
  pubSubClient.publish("student/CASA0014/plant/ucfnawe/moisture", msg);

}

///
/// This will print the message to the serial and trigger an LED to match the
/// first character (i.e. on if the first character is 1) for subscribed-to payloads.
///
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

///
/// Create a connection to the MQTT server, subscribing to the temp., humidity, and moisture topics
/// from this particular plant monitor.
///
void reconnect() {
  // Loop until we're reconnected
  while (!pubSubClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID so as to avoid accidental overlaps between clients
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect with clientID, username and password
    if (pubSubClient.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
      // ... and resubscribe
      pubSubClient.subscribe("student/CASA0014/plant/ucfnawe/temperature");
      pubSubClient.subscribe("student/CASA0014/plant/ucfnawe/humidity");
      pubSubClient.subscribe("student/CASA0014/plant/ucfnawe/moisture");
    } else {
      Serial.print("failed, rc=");
      Serial.print(pubSubClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

///
/// Collects temperature and humidity reading, collect and (along with most recent moisture reading) 
/// sends to server for viewing by web browser
///
void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  server.send(200, "text/html", SendHTML(Temperature, Humidity, Moisture));
}

///
/// 404 error if handle has not been defined.
///
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

///
/// HTML markdown for viewing the readings from a web browser
///
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
  ptr += "<h1>ESP8266 Huzzah DHT22 Report</h1>\n";

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
  ptr += timeZone.dateTime("l,");
  ptr += "<br>";
  ptr += timeZone.dateTime("d-M-y H:i:s T");
  ptr += "</p>";

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
