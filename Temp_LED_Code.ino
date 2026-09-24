#include <Wire.h>
#include <BH1750.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>  // LCD library

// Pin Definitions
#define ONE_WIRE_BUS 2    // DS18B20 data pin connected to D2
#define LED_PIN 3         // LED connected to D3 (PWM capable)

// Sensor and Display Objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
BH1750 lightMeter;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Check address: 0x27 or 0x3F for your LCD

void setup() {
  Serial.begin(9600);
  Wire.begin();

  sensors.begin();
  lightMeter.begin();
  lcd.init();          // Initialize LCD
  lcd.backlight();     // Turn on backlight

  pinMode(LED_PIN, OUTPUT);

  Serial.println("System Initialized: DS18B20 + BH1750 + LCD + Adaptive LED");
}

void loop() {
  // Read Temperature
  sensors.requestTemperatures();
  float temperatureC = sensors.getTempCByIndex(0);

  // Read Light Intensity
  float lux = lightMeter.readLightLevel();

  // Print readings to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperatureC);
  Serial.print(" *C | Light: ");
  Serial.print(lux);
  Serial.println(" lux");

  // Adaptive LED Lighting based on Lux
  if (lux < 30) {
    analogWrite(LED_PIN, 255);   // Very dark: Full brightness
  } 
  else if (lux >= 30 && lux <= 100) {
    analogWrite(LED_PIN, 100);   // Medium light: Dim brightness
  } 
  else {
    analogWrite(LED_PIN, 0);     // Very bright: LED OFF
  }

  // Display on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(temperatureC, 1); // 1 decimal place
  lcd.print((char)223);       // Degree symbol
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Light:");
  lcd.print(lux, 0);          // 0 decimal places
  lcd.print(" lux");

  delay(1000); // 1 second delay between updates
}
