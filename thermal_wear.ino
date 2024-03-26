#include <LiquidCrystal.h>

// Initialize the LCD with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// Add a variable to keep track of the display mode
bool displayTemperatures = true;
unsigned long lastSwitchTime = 0;
const unsigned long switchInterval = 200; // Switch every 3000 milliseconds (3 seconds)

// Global variables
int roomTempPin = A0; // Room temperature sensor connected to A0
int bodyTempPin = A1; // Body temperature sensor connected to A1
int heaterPin = 9;    // Heater (LED) connected to pin 9
int potPin = A2;      // Potentiometer connected to A2

// Define the temperature thresholds for the different heating modes
const float mildHeatBodyThreshold = 36.0; // Body temperature threshold for mild heat
const float intermediateHeatBodyThreshold = 34.0; // Body temperature threshold for intermediate heat
const float highHeatBodyThreshold = 32.0; // Body temperature threshold for high heat

// Define base PWM values for the different heating modes
const int mildHeatBaseIntensity = 85; // Base PWM value for mild heat
const int intermediateHeatBaseIntensity = 170; // Base PWM value for intermediate heat
const int highHeatBaseIntensity = 255; // Base PWM value for high heat

// Define the influence of room temperature. A lower room temperature increases the intensity.
const float roomTempInfluence = 0.5; // Determines how much the room temperature influences the heater intensity

// Define the safety cutoff temperature threshold
const float safetyCutoffThreshold = 38.0; // Body temperature threshold for safety cutoff

void setup() {
  // Set up the pins
  pinMode(roomTempPin, INPUT);
  pinMode(bodyTempPin, INPUT);
  pinMode(heaterPin, OUTPUT);
  pinMode(potPin, INPUT);

  // Set up the LCD's number of columns and rows
  lcd.begin(16, 2);

  // Start with a clear screen
  lcd.clear();

  // Display a welcome message
  lcd.print("Thermal Wear!");

  // Start the serial communication
  Serial.begin(9600);
}

void loop() {
  int roomTemp = analogRead(roomTempPin);
  int bodyTemp = analogRead(bodyTempPin);

  // Convert analog reading to voltage
  float roomTempVoltage = (roomTemp * 5.0) / 1023.0;
  float bodyTempVoltage = (bodyTemp * 5.0) / 1023.0;

  // Convert voltage to temperature
  float roomTempCelsius = (roomTempVoltage - 0.5) * 100.0;
  float bodyTempCelsius = (bodyTempVoltage - 0.5) * 100.0;

  // Read the potentiometer value
  int manualHeaterIntensity = analogRead(potPin);
  manualHeaterIntensity = map(manualHeaterIntensity, 0, 1023, 0, 255);

  // Heater control logic
  int heaterIntensity = 0;
  String heatMode = "Off"; // Default mode

  if (bodyTempCelsius > safetyCutoffThreshold) {
    heaterIntensity = 0;
    heatMode = "Safety Off";
    Serial.println("Safety Cutoff Activated - Heater OFF");
  } else {
    float roomTempModifier = (25.0 - roomTempCelsius) * roomTempInfluence;
    if (bodyTempCelsius < highHeatBodyThreshold) {
      heaterIntensity = highHeatBaseIntensity;
      heatMode = "High Heat";
    } else if (bodyTempCelsius < intermediateHeatBodyThreshold) {
      heaterIntensity = intermediateHeatBaseIntensity;
      heatMode = "Medium Heat";
    } else if (bodyTempCelsius < mildHeatBodyThreshold) {
      heaterIntensity = mildHeatBaseIntensity;
      heatMode = "Mild Heat";
    }

    heaterIntensity += roomTempModifier;
    heaterIntensity = constrain(heaterIntensity, 0, 255);
    
    if (abs(manualHeaterIntensity - heaterIntensity) > 70) { 
      heaterIntensity = manualHeaterIntensity;
      Serial.println("Manual Override Active");
    }
  }
  
  // Decide on the heater intensity based on body temperature
  if (bodyTempCelsius < highHeatBodyThreshold) {
    heaterIntensity = highHeatBaseIntensity;
    Serial.println("High Heat");
  } else if (bodyTempCelsius < intermediateHeatBodyThreshold) {
    heaterIntensity = intermediateHeatBaseIntensity;
    Serial.println("Intermediate Heat");
  } else if (bodyTempCelsius < mildHeatBodyThreshold) {
    heaterIntensity = mildHeatBaseIntensity;
    Serial.println("Mild Heat");
  }

  // Apply the heater intensity
  analogWrite(heaterPin, heaterIntensity);

  // Output the heater intensity and temperature readings to the serial monitor
  Serial.print("Room Temp: ");
  Serial.print(roomTempCelsius);
  Serial.print(" C, Body Temp: ");
  Serial.print(bodyTempCelsius);
  Serial.println(" C");
  Serial.print("Heater Intensity: ");
  Serial.println(heaterIntensity);
  Serial.print("Heater Mode: ");
  Serial.println(heatMode);
  
  // Check for temperature changes and update only if there's a change
  static float lastRoomTempCelsius = 0;
  static float lastBodyTempCelsius = 0;
  static String lastHeatMode = "";

  if (lastRoomTempCelsius != roomTempCelsius ||
      lastBodyTempCelsius != bodyTempCelsius ||
      lastHeatMode != heatMode) {
    // Store the current temperatures and mode for comparison in the next loop iteration
    lastRoomTempCelsius = roomTempCelsius;
    lastBodyTempCelsius = bodyTempCelsius;
    lastHeatMode = heatMode;
    
  // Check if it's time to switch the display mode
  if (millis() - lastSwitchTime >= switchInterval) {
    // It's time to switch the display mode
    displayTemperatures = !displayTemperatures;
    lastSwitchTime = millis(); // Update the last switch time
    lcd.clear(); // Clear the display for new information
  }

  // Decide what to display based on the current mode
  if (displayTemperatures) {
    // Display the temperatures
    lcd.setCursor(0, 0); // First line for room temperature
    lcd.print("Room: ");
    lcd.print(roomTempCelsius);
    lcd.print(" C ");

    lcd.setCursor(0, 1); // Second line for body temperature
    lcd.print("Body: ");
    lcd.print(bodyTempCelsius);
    lcd.print(" C ");
  } else {
    // Display the heater mode
    lcd.setCursor(0, 0); // Use both lines for heater mode
    lcd.print("Mode: ");
    lcd.print(heatMode);
  }
  }
  delay(100); // Wait for a second

}
  