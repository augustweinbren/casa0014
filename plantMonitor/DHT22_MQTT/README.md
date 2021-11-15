## Adapting DHT22_MQTT to a different monitor
The DHT22_MQTT script requires a few changes to adapt it to a different monitor:
1. Ensure that `continentCity` matches the time zone where the device is in use.
2. Ensure that the `mqtt_server` set to the server you intend to use.
3. Set `Moisture=map(Moisture, ...)` to match empirical observations.
4. Set first argument of `pubSubClient.publish()` functions to match the desired topic.
5. Set arguments of `pubSubClient.subscribe()` lines to match those of the `pubSubClient.publish()` lines.