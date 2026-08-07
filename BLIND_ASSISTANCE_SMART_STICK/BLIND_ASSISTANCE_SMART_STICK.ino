#include <HardwareSerial.h>
#include <TinyGPS++.h>

//PIN DEFINITIONS 

// Ultrasonic sensor
int trigPin = 5;
int echoPin = 18;

// Water sensor
int waterSensor = 34;

// Alerts
int buzzer = 25;
int vibMotor = 26;

// SOS button
int sosButton = 27;

// GPS
int gpsRx = 16;
int gpsTx = 17;

// GSM
int gsmRx = 4;
int gsmTx = 2;


//GPS AND SERIAL

TinyGPSPlus gps;

HardwareSerial gpsSerial(1);
HardwareSerial gsmSerial(2);


//SETTINGS
// Obstacle distance in centimeters
int obstacleDistance = 50;

// Water sensor threshold
int waterThreshold = 1500;


//SETUP

void setup() {

  Serial.begin(115200);

  // Ultrasonic
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Water sensor
  pinMode(waterSensor, INPUT);

  // Alert devices
  pinMode(buzzer, OUTPUT);
  pinMode(vibMotor, OUTPUT);

  // SOS button
  pinMode(sosButton, INPUT_PULLUP);

  // Start GPS
  gpsSerial.begin(9600, SERIAL_8N1, gpsRx, gpsTx);

  // Start GSM
  gsmSerial.begin(9600, SERIAL_8N1, gsmRx, gsmTx);

  // Turn alerts OFF
  digitalWrite(buzzer, LOW);
  digitalWrite(vibMotor, LOW);

  Serial.println();
  Serial.println("Blind Assistance Smart Stick");
  Serial.println("System Started");
}


//MAIN LOOP 
void loop() {

  //GPS DATA

  while (gpsSerial.available()) {
    gps.encode(gpsSerial.read());
  }


  //ULTRASONIC 

  int distance = getDistance();


  //WATER SENSOR

  int waterVal = analogRead(waterSensor);


  //SERIAL MONITOR 

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm");

  Serial.print(" | Water: ");
  Serial.println(waterVal);


  //OBSTACLE / WATER ALERT 

  if ((distance > 0 && distance <= obstacleDistance) ||
      waterVal > waterThreshold) {

    digitalWrite(buzzer, HIGH);
    digitalWrite(vibMotor, HIGH);

    Serial.println("WARNING: Obstacle or water detected!");

  } 
  else {

    digitalWrite(buzzer, LOW);
    digitalWrite(vibMotor, LOW);
  }


  //SOS BUTTON 

  if (digitalRead(sosButton) == LOW) {

    Serial.println("SOS BUTTON PRESSED!");

    sendSOS();

    // Prevent repeated SMS messages
    delay(5000);
  }


  delay(150);
}


//ULTRASONIC FUNCTION
int getDistance() {

  // Make sure trigger is LOW
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Send ultrasonic pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure echo
  long duration = pulseIn(echoPin, HIGH, 30000);

  // If no echo is received
  if (duration == 0) {
    return 0;
  }

  // Calculate distance
  int distance = duration * 0.034 / 2;

  return distance;
}


//SOS FUNCTION
void sendSOS() {

  Serial.println("Preparing SOS alert...");


  // GET GPS LOCATION

  String locationMessage;

  if (gps.location.isValid()) {

    String latitude = String(gps.location.lat(), 6);
    String longitude = String(gps.location.lng(), 6);

    locationMessage = "https://maps.google.com/?q=";
    locationMessage += latitude;
    locationMessage += ",";
    locationMessage += longitude;

    Serial.print("Latitude: ");
    Serial.println(latitude);

    Serial.print("Longitude: ");
    Serial.println(longitude);

  }
  else {

    locationMessage = "GPS location not available";

    Serial.println("GPS location not available");
  }


  //GSM SMS

  gsmSerial.println("AT");
  delay(1000);

  gsmSerial.println("AT+CMGF=1");
  delay(1000);


  // CHANGE THIS ONLY WHEN TESTING LOCALLY
  // Do NOT put your real number in a public GitHub repository.

  gsmSerial.println("AT+CMGS=\"YOUR_PHONE_NUMBER\"");
  delay(1000);


  // SMS message

  gsmSerial.println("SOS ALERT!");

  gsmSerial.println("Emergency assistance may be required.");

  gsmSerial.print("Location: ");
  gsmSerial.println(locationMessage);


  // CTRL + Z tells SIM800L to send the SMS

  gsmSerial.write(26);

  delay(5000);

  Serial.println("SOS message command sent.");
}
