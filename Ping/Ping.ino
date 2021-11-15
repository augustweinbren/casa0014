/*
  Ping))) Sensor

  This sketch reads a PING))) ultrasonic rangefinder and returns the distance
  to the closest object in range. To do this, it sends a pulse to the sensor to
  initiate a reading, then listens for a pulse to return. The length of the
  returning pulse is proportional to the distance of the object from the sensor.

  The circuit:
	- +V connection of the PING))) attached to +5V
	- GND connection of the PING))) attached to ground
	- SIG connection of the PING))) attached to digital pin 7

  created 3 Nov 2008
  by David A. Mellis
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Ping
*/
#include <Wire.h>

const int trigPin = 9;
const int echoPin = 10;
const int LED = 13;
const boolean debug = true;

long duration, cm;
int blinkrate = 0;

// this constant won't change. It's the pin number of the sensor's output:
const int pingPin = 7;

void setup() {
  // initialize serial communication:
  if(debug){
    Serial.begin(9600);
    Serial.println("Starting blinker");
  }
  pinMode(LED, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  cm = microsecondsToCentimeters(duration);

  if(cm < 100){
    if(cm < 30){
      blinkrate = map(cm, 1, 30, 10, 300);
    }
    else if(cm < 90){
      blinkrate = map(cm, 30, 90, 300, 1000);
    }
    else{
      blinkrate = 0;
    }

    if(debug){
      Serial.print(" cm: ");
      Serial.print(cm);
      Serial.print("  blinkrate: ");
      Serial.println(blinkrate);
    }
    digitalWrite(LED, HIGH);
    delay(blinkrate);
    digitalWrite(LED, LOW);
    delay(blinkrate);
  }
  delay(20);
}

long microsecondsToCentimers(long microseconds) {
  return microseconds / 29 / 2;
}
long microsecondsToInches(long microseconds) {
  // According to Parallax's datasheet for the PING))), there are 73.746
  // microseconds per inch (i.e. sound travels at 1130 feet per second).
  // This gives the distance travelled by the ping, outbound and return,
  // so we divide by 2 to get the distance of the obstacle.
  // See: https://www.parallax.com/package/ping-ultrasonic-distance-sensor-downloads/
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the object we
  // take half of the distance travelled.
  return microseconds / 29 / 2;
}
