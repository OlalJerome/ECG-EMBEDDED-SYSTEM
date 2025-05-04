#include <Arduino.h>

// Pin Configuration
const int ECG_PIN = 33;
const int LED_PIN = 13;
const int ELECTRODE1_PIN = 4;
const int ELECTRODE2_PIN = 5;

// Sampling Configuration
const int SAMPLE_RATE = 100;
const int FILTER_WINDOW = 5;

// Filter variables
float filter_buffer[FILTER_WINDOW];
int filter_index = 0;
float baseline = 2048.0;  // Initial baseline (midpoint of 12-bit ADC)

// Status variables
bool electrodes_connected = false;
bool inverted = false;

float movingAverage(float new_value) {
  filter_buffer[filter_index] = new_value;
  filter_index = (filter_index + 1) % FILTER_WINDOW;
  
  float sum = 0;
  for (int i = 0; i < FILTER_WINDOW; i++) {
    sum += filter_buffer[i];
  }
  return sum / FILTER_WINDOW;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  pinMode(ELECTRODE1_PIN, INPUT_PULLUP);
  pinMode(ELECTRODE2_PIN, INPUT_PULLUP);
  
  // Initialize filter buffer
  for (int i = 0; i < FILTER_WINDOW; i++) {
    filter_buffer[i] = baseline;
  }
}

void loop() {
  // Read electrode contact status
  bool contact1 = digitalRead(ELECTRODE1_PIN);
  bool contact2 = digitalRead(ELECTRODE2_PIN);
  
  // Send electrode status
  Serial.print("ELECTRODES,");
  Serial.print(contact1 ? "1" : "0");
  Serial.print(",");
  Serial.println(contact2 ? "1" : "0");

  // Update connection status
  bool new_connection = contact1 && contact2;
  if (new_connection != electrodes_connected) {
    electrodes_connected = new_connection;
    Serial.print("STATUS,");
    Serial.println(electrodes_connected ? "1" : "0");
    digitalWrite(LED_PIN, electrodes_connected ? HIGH : LOW);
  }

  if (electrodes_connected) {
    int raw = analogRead(ECG_PIN);
    
    // Handle signal inversion
    if (abs(raw - baseline) > 500) {
      inverted = (raw < baseline);
    }
    if (inverted) raw = 4095 - raw;
    
    // Apply filtering
    float filtered = movingAverage(raw);
    
    // Send data packet
    Serial.print("DATA,");
    Serial.print(raw);
    Serial.print(",");
    Serial.print(filtered);
    Serial.print(",");
    Serial.println(inverted ? "1" : "0");
  }

  delay(1000 / SAMPLE_RATE);
}
