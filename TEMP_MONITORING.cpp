#include <Wire.h>
#include <hd44780.h> // Main hd44780 header
#include <hd44780ioClass/hd44780_I2Cexp.h> // I2C expander I/O class header


// Constants for the calculation
const float R2 = 250.0;  // Value of the 10k resistor (R2) in ohms
const float Vcc = 3.340;     // Supply voltage (3.3V for Arduino)
const float Aref = 5.02;    // Actual reference voltage measured (5.1V in this case)
const int analogPin = A3;  // Analog pin for voltage measurement
const int numReadings = 50;  // Number of readings for averaging (1 second @ 50Hz)


// Calibration Factor (Adjust this as needed for your sensor setup)
float calibrationFactor = 0.9725;  // Default no calibration, modify for your system


// Arrays to store readings for averaging
int readings[numReadings];  // Array to store the readings
int readIndex = 0;  // Index for the current reading
long total = 0;  // Running total of the readings
int averageValue = 0;  // Averaged value
unsigned long previousMillis = 0;  // Stores the last time we printed the value
const long interval = 1000;  // Interval to print the value (1 second)


// Initialize the LCD
hd44780_I2Cexp lcd;  // Create an lcd object using the I2C interface


void setup() {
  // Initialize Serial Monitor for output
  Serial.begin(9600);


  // Initialize readings array
  for (int i = 0; i < numReadings; i++) {
    readings[i] = 0;  // Initialize all readings to 0
  }


  // Initialize the LCD
  lcd.begin(16, 2); // Initialize LCD with 16 columns and 2 rows
  lcd.clear();  // Clear the screen to prepare for displaying data
}


void loop() {
  // Read the current analog value
  int rawValue = analogRead(analogPin);


  // Subtract the last reading
  total = total - readings[readIndex];


  // Store the new reading
  readings[readIndex] = rawValue;


  // Add the new reading to the total
  total = total + readings[readIndex];


  // Advance to the next index in the array
  readIndex = readIndex + 1;


  // If we've reached the end of the array, start over
  if (readIndex >= numReadings) {
    readIndex = 0;
  }


  // Calculate the average value from the last 50 readings
  averageValue = total / numReadings;


  // Get the current time in milliseconds
  unsigned long currentMillis = millis();


  // If one second has passed, print the average temperature, resistance, and voltage
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;  // Update the last time printed


    // Calculate the PT100 resistance from the averaged value
    float pt100Resistance = calculatePT100Resistance(averageValue);


    // Apply calibration factor (if any)
    pt100Resistance *= calibrationFactor;


    // Calculate the temperature from the resistance
    float temperature = calculateTemperature(pt100Resistance);


    // Clear the LCD and print the PT100 resistance and temperature
    lcd.clear();  // Clear the LCD screen
    lcd.setCursor(0, 0);  // Set cursor to the first row, first column
    lcd.print("Res: ");  // Display label for resistance
    lcd.print(pt100Resistance, 1);  // Display resistance with one decimal place
    lcd.print(" ohm");  // Display units for resistance


    lcd.setCursor(0, 1);  // Move to the second row
    lcd.print("Temp: ");  // Display label for temperature
    lcd.print(temperature, 1);  // Display temperature with one decimal place
    lcd.print(" C");  // Display temperature unit


    // Also output the values to the Serial Monitor
    Serial.print("PT100 Resistance: ");
    Serial.print(pt100Resistance);
    Serial.print(" ohms\t Temperature: ");
    Serial.print(temperature);
    Serial.println(" C");
  }
}


// Function to calculate resistance of PT100
float calculatePT100Resistance(int rawValue) {
  // Convert raw analog value to voltage (0-1023 -> 0 - 5V)
  float voltage = rawValue * (Aref / 1023.0);


  // Use voltage divider formula to calculate PT100 resistance
  float PT100Resistance = R2 * ((Vcc / voltage) - 1);


  return PT100Resistance;
}


// Function to calculate temperature from PT100 resistance
float calculateTemperature(float resistance) {
  // PT100 temperature coefficient
  const float alpha = 0.00385;  // Temperature coefficient in ohms per °C


  // Use the linear approximation formula for PT100 temperature
  float temperature = (resistance - 100) / (alpha * 100);


  return temperature;
}
