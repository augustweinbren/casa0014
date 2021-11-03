# Plant Monitor
Second workshop for CASA0014 - 2 - Plant Monitor

## File descriptions
This directory contains all of the code needed to run a DHT22 and nail-based plant monitor built on a Feather Huzzah microcontroller using an ESP8266 wifi module.
In particular, humidity, temperature, and moisture readings are collected and sent to an MQTT server and a web server (for testing purposes).

The script which can be uploaded to the microcontroller to perform the aforementioned sensing and publishing can be found in the `DHT22_MQTT` repository.

Additionally, the following test scripts can be uploaded to test the different components separately:
1. testHTTP: this will test that the board is able to connect to WiFi using the `ESP8266WiFi` library, by attempting to get data from `iot.io/data/index.html`.
2. testEZtime: Using the `ezTime` library, this will test that the board is able to connect to the NTP server and retrieve and store a date time value
3. testMQTT: Using the `PubSubClient` library, this tests the MQTT calls of publish and subscribe on `mqtt.ce.org`. Specifically, it includes a functions which turns an on-board LED if the first character in a subscribed-to message is 1, and turns it off if the character is different.
4. testEnvWeb: this will test that the DHT22 sensor data will send to a web server. The associated web server can be accessed by copying the IP address from the serial output and appending `:80`.
5. testMoisture: this will test that the nail-based soil sensor can measure moisture in the soil. It prints the moisture values directl
N.B. running files 1-4 requires either editing the test scripts themselves or adding an `ArduinoSecrets.h` file containing the SSID and password for the WiFi network.

## Adapting DHT22_MQTT to a different monitor
The DHT22_MQTT script requires a few changes to adapt it to a different monitor:
1. Ensure that `continentCity` matches the time zone where the device is in use.
2. Ensure that the `mqtt_server` set to the server you intend to use.
3. Set `Moisture=map(Moisture, ...)` to match empirical observations.
4. Set first argument of `pubSubClient.publish()` functions to match the desired topic.
5. Set arguments of `pubSubClient.subscribe()` lines to match those of the `pubSubClient.publish()` lines.

## Viewing data from the plant monitor
Data can be viewed by opening the `MQTT Explorer` client. Create a connection to `mqtt.cetools.org`. If this plant monitor is currently collecting measurements, you should see data in the topic `student/CASA0014/plant/ucfnawe`.
