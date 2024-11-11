#include "DHT.h"
#include "HX711.h"

// Pin Definitions
#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Motor Driver Pin Definitions
const int motorIN1 = 2;
const int motorIN2 = 3;
const int motorEnable = 9;

// LPG Gas Sensor Pin
const int gasPin = A0;

// HX711 Load Cell Pins
const int LOADCELL_DOUT_PIN = 6;  
const int LOADCELL_SCK_PIN = 7;   

HX711 scale;

// Thresholds
const int GAS_THRESHOLD = 400; 
const float TEMP_THRESHOLD = 30.0; // Temperature threshold for fan 
const float HUMIDITY_THRESHOLD = 70.0; // Humidity threshold for fan 
const long EMPTY_WEIGHT_THRESHOLD = 50; // Weight threshold for cylinder

// Function to send notifications via Serial Monitor
void notify(const char* message) {
  // Serial.println(message);
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);

  // Initialize DHT sensor
  dht.begin();

  // Initialize HX711 Load Cell
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  
  scale.set_scale(2280.f);  
  
  // reset the scale to zero
  scale.tare();  
  Serial.println("Taring the load cell...");
  delay(1000);  
  
  // Motor driver setup
  pinMode(motorIN1, OUTPUT);
  pinMode(motorIN2, OUTPUT);
  pinMode(motorEnable, OUTPUT);

  // Gas sensor setup
  pinMode(gasPin, INPUT);

  // Ensure the fan is initially off
  digitalWrite(motorIN1, LOW);
  digitalWrite(motorIN2, LOW);
  analogWrite(motorEnable, 0);
}

void loop() {
  // LPG Gas Sensor (Leak Detection)
  int gasLevel = analogRead(gasPin);
  bool gasLeakDetected = gasLevel > GAS_THRESHOLD;
  String gasStatus = (gasLevel > GAS_THRESHOLD) ? "Leak Detected" : "Safe";

  // Temperature and Humidity Monitoring (Fan Control)
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  // Check if the DHT sensor is returning valid data
  if (isnan(temp) || isnan(hum)) {
    notify("Error: Unable to read temperature/humidity from DHT11.");
  } else {
    // Fan Control Logic: Turn on the fan if temperature/humidity exceeds threshold but NOT if a gas leak is detected
    if ((temp > TEMP_THRESHOLD || hum > HUMIDITY_THRESHOLD) && !gasLeakDetected) {
      // Turn on the fan
      digitalWrite(motorIN1, HIGH);
      digitalWrite(motorIN2, LOW);
      analogWrite(motorEnable, 255); // Full speed
      notify("Fan ON: High temperature/humidity detected.");
    } else {
      // Turn off the fan
      digitalWrite(motorIN1, LOW);
      digitalWrite(motorIN2, LOW);
      analogWrite(motorEnable, 0); // Stop fan
      notify("Fan OFF: Kitchen Conditions is normal.");
    }
  }

  // Read the weight from the load cell
  long weight = scale.get_units(50);  

  // Check if the cylinder is empty
  // if (weight < EMPTY_WEIGHT_THRESHOLD) {
  //   notify("Cylinder is empty! Please refill.");
  // } else {
  //   notify("Cylinder has sufficient weight.");
  // }
  
  String output = "Weight:" + String(0) + " kg | " +
                   "Gas:" + gasStatus + " | " +
                   "Temperature:" + String(temp, 1) + " °C | " +
                   "Humidity:" + String(hum, 1) + " %";
  Serial.println(output);

  // Add delay to avoid spamming notifications
  delay(2000);
}
