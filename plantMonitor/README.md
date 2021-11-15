**Table of Contents**
- [Description <a name="description"></a>](#description-)
- [Items Needed <a name="itemsNeeded"></a>](#items-needed-)
- [Installations <a name="installation"></a>](#installations-)
- [Adapting and running examples to ensure board is working <a name="tDDArduino"></a>](#adapting-and-running-examples-to-ensure-board-is-working-)
- [Circuit Prototyping <a name="circuitry"></a>](#circuit-prototyping-)
- [Raspberry Pi-based data collection and visualisation <a name = "rpi"></a>](#raspberry-pi-based-data-collection-and-visualisation-)

## Description <a name="description"></a>
This directory contains all of the code needed to run a DHT22 and nail-based plant monitor built on a Feather Huzzah microcontroller using an ESP8266 wifi module.
In particular, humidity, temperature, and moisture readings are collected and sent to an MQTT server and a web server (for testing purposes).

The script which can be uploaded to the microcontroller to perform the aforementioned sensing and publishing can be found in the `DHT22_MQTT` repository. Please refer to the README files in the test script directories for descriptions of each script.

## Items Needed <a name="itemsNeeded"></a>
- Feather Huzzah ESP8266 WiFi Board
- Raspberry Pi
- 2 Nails
- Jumper cables
- Additional cabling for the nails + electrical tape (say .5m per nail)
- Plant for monitoring
- (Optional) PN222 Transistor
- DHT22 Temperature-Humidity sensor
- Resistors

## Installations <a name="installation"></a>
1.	Download and install [Arduino IDE](https://www.arduino.cc/en/software) and [MQTT Explorer](https://github.com/thomasnordquist/MQTT-Explorer/releases)
2.	[Set up a GitHub repo](https://docs.github.com/en/get-started/quickstart/create-a-repo) for the project
3.	Launch Arduino IDE and Huzzah into USB port
4.	[Add board via board manager](https://learn.adafruit.com/adafruit-feather-huzzah-esp8266/using-arduino-ide) if unrecognised



## Adapting and running examples to ensure board is working <a name="tDDArduino"></a>
1.	Deploy the [blink example](https://www.arduino.cc/en/Tutorial/BuiltInExamples/Blink) to ensure the board is working
2.	The [testHTTP](https://github.com/augustweinbren/casa0014/tree/main/plantMonitor/testHTTP) code can be used to test the WiFi connection on the Arduino board. Fill in the ssid, password to match your WiFi network. 
3.	Upload the code to the board (using the right arrow in the top left) and open the Serial Monitor (at the top right) to check that the device is able to connect to WiFi.and the web client.
4.	Test that the board can get the time data from the internet. Edit the [testEztime](https://github.com/augustweinbren/casa0014/tree/main/plantMonitor/testEZtime) code, again by filling in the ssid and password, and again upload the script to the board and open the Serial monitor.
5.	Test that the MQTT calls of publish and subscribe can be sent by the board with [testMQTT](https://github.com/augustweinbren/casa0014/tree/main/plantMonitor/testMQTT). Create an `arduino_secrets.h` file populated using the syntax described in `testMQTT`. [Ensure that this file is ignored by Git](https://docs.github.com/en/get-started/getting-started-with-git/ignoring-files). Search for the “client.publish” and “client.subscribe” lines and change the topic arguments if you wish. Again upload the file to the board.
6.	To test this source code, open up MQTT Explorer, creating a connection to the mqtt.cetools.org host under port 1884. Type in the same username and password as you entered into the secrets file. You should observe a “hello world” message that is listed under the topic which you set in the previous step.
7. To test that the "subscribe" functionality is working properly, use the "publish" pane of MQTT Explorer. Fill in "Topic" with the argument in the client.subscribe command of the reconnect() method, select "raw" as the data type, and type a 1 in the box. Click "publish". The built-in LED on your board should light up. If you type 0 in the box and click "publish" again, the LED should turn off.
## Circuit Prototyping <a name="circuitry"></a>
1. To integrate both the moisture sensor and the DHTT sensor together requires a complex circuit:
<p>
  <img alt="Moisture sensor circuit powered by digital pin" src="./img/finalDesign.png">
    <em>Plant monitor: integrated DHT22 and moisture sensor (Wilson, 2021)</em>
</p>

[N.B. a slightly simplified version using digital output instead of the transistor as a switch is currently in progress]

2. Test that your sensor is giving you sensible data by running the `testMoisture` script. Change the values as listed in the GitHub README.

3. Now make the changes to `DHT22_MQTT` as described in its README.

## Raspberry Pi-based data collection and visualisation <a name = "rpi"></a>

1. Download the Raspberry Pi Imager [https://www.raspberrypi.com/software/], and set up the device using the following tutorial on a MicroSD card plugged into your computer: [https://www.tomshardware.com/uk/reviews/raspberry-pi-headless-setup-how-to,6028.html]. Choose a meaningful hostname.
2. Insert card into a computer and type: `ssh pi@[hostname].local`
3. Bring all files up to date: `sudo apt update; sudo apt upgrade -y; reboot`
4. Add InfluxDB key: `wget -qO- https://repos.influxdata.com/influxdb.key | sudo apt-key add -`
5. Store debian repository in sources list, update and install InfluxDB: `echo "deb https://repos.influxdata.com/debian buster stable" | sudo tee /etc/apt/sources.list.d/influxdb.list ; sudo apt update; sudo apt install influxdb`
6. Get influx started at reboot: `sudo systemctl unmask influxdb.service; sudo systemctl start influxdb; sudo systemctl enable influxdb.service`
7. Open the InfluxDB CLI: `influx`
8. Type in the below queries:
> CREATE DATABASE telegraf
> USE telegraf
> CREATE USER admin WITH PASSWORD 'admin' WITH ALL PRIVILEGES
> GRANT ALL PRIVILEGES ON telegraf TO admin
> SHOW USERS
> exit
9. Download telegraf: `wget https://dl.influxdata.com/telegraf/releases/telegraf_1.19.2-1_armhf.deb` [Note: use a different version depending on the Raspberry pi you are using]
10. Install: `sudo dpkg -i telegraf_1.19.2-1_armhf.deb`
11. Edit `/etc/influxdb/influxdb.conf` with root-privileges so that `enabled = true`, `bind-address = ":8086"` and `auth-enabled = true`.
12. Restart influxdb: 
`sudo systemctl stop influxdb;
sudo systemctl start influxdb`
13. Install grafana to allow data visualisation: 
>sudo apt-get install -y adduser libfontconfig1
wget https://dl.grafana.com/oss/release/grafana_8.1.1_armhf.deb
sudo dpkg -i grafana_8.1.1_armhf.deb
14. Get grafana running:
>sudo /bin/systemctl daemon-reload
sudo /bin/systemctl enable grafana-server
sudo /bin/systemctl start grafana-server
15. Going to `hostname.local:3000`, you should see the Grafana website. Insert a default username and password of `admin admin`.
16. Replace the telegraf configuration with a simpler file: `cd /etc/telegraf ; sudo systemctl stop telegraf ; sudo mv telegraf.conf telegraf.conf.backup ; sudo wget https://raw.githubusercontent.com/ucl-casa-ce/casa0014/main/plantMonitor/pi/etc/telegraf/telegraf.conf`
17. Edit the `servers` and `topics` to match those where your data is getting published to.
18. Start telegraf and check for errors. 
> sudo systemctl start telegraf.service
sudo systemctl status telegraf.service
19. Add your first data source by following the onscreen prompts, select `InfluxDB`, add `localhost:8086` at the URL, add `telegraf` as the database and fill out your username and password, click `save and test`
20. Create a dashboard from the '+' symbol on the left sidebar. Click 'Empty Panel'.
21. From the Query panel, select `mqtt_consumer` in the `From` row. It should appear after typing a few letters.
22. Now in the `GROUP BY` row, select `tag(topic)`. This will split temperature, humidity, and moisture.