#include <Wire.h>
#include <BH1750.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// === Pin Definitions ===
const int TDS_PIN = A1;       // TDS analog input
const int TEMP_PIN = 2;       // DS18B20 data pin
const int LIGHT_PIN = 3;      // PWM output to ambient light

// === Sensor Objects ===
BH1750 lightMeter;
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(TEMP_PIN);
DallasTemperature tempSensor(&oneWire);

// === Variables ===
float prevTemp = 0, prevTDS = 0;
unsigned long lastDisplayTime = 0;
int displayState = 0; // 0 = light, 1 = temp, 2 = tds

void setup() {
  Wire.begin();
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.backlight();
  lightMeter.begin();
  tempSensor.begin();
  pinMode(LIGHT_PIN, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Smart Aquarium");
  delay(2000);
  lcd.clear();
}

void loop() {
  // === Read Sensors ===
  float lux = lightMeter.readLightLevel();
  float temperature = getTemperature();
  float tds = readTDS();

  // === Update Ambient Light ===
  int brightness = map((int)lux, 0, 500, 255, 0);
  brightness = constrain(brightness, 0, 255);
  analogWrite(LIGHT_PIN, brightness);

  // === Display Data Cyclically ===
  if (millis() - lastDisplayTime > 3000) {  // Change display every 3 seconds
    lcd.clear();
    if (displayState == 0) {
      lcd.setCursor(0, 0);
      lcd.print("Light: ");
      lcd.print((int)lux);
      lcd.print(" lx");

      lcd.setCursor(0, 1);
      lcd.print("PWM: ");
      lcd.print(brightness);

    } else if (displayState == 1) {
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temperature);
      lcd.print(" C");

      lcd.setCursor(0, 1);
      if (temperature > prevTemp + 2) {
        lcd.print("Spike in Temp!");
      } else if (temperature < 20) {
        lcd.print("Too Cold");
      } else if (temperature > 35) {
        lcd.print("Too Hot");
      } else {
        lcd.print("Optimal");
      }
      prevTemp = temperature;

    } else if (displayState == 2) {
      lcd.setCursor(0, 0);
      lcd.print("TDS: ");
      lcd.print((int)tds);
      lcd.print(" ppm");

      lcd.setCursor(0, 1);
      if (tds > prevTDS + 100) {
        lcd.print("TDS Spike!");
      } else if (tds > 600) {
        lcd.print("Hard Water");
      } else if (tds < 150) {
        lcd.print("Soft Water");
      } else {
        lcd.print("Normal");
      }
      prevTDS = tds;
    }

    displayState = (displayState + 1) % 3;
    lastDisplayTime = millis();
  }

  // Optional: Serial debug output for MATLAB later
  Serial.print("Lux:");
  Serial.print(lux);
  Serial.print(",Temp:");
  Serial.print(temperature);
  Serial.print(",TDS:");
  Serial.println(tds);
}

// === Read Temperature from DS18B20 ===
float getTemperature() {
  tempSensor.requestTemperatures();
  return tempSensor.getTempCByIndex(0);
}

// === Read TDS value (basic analog scaling) ===
float readTDS() {
  int raw = analogRead(TDS_PIN);
  float voltage = raw * (5.0 / 1024.0);
  float tdsValue = (133.42 * voltage * voltage * voltage
                   - 255.86 * voltage * voltage
                   + 857.39 * voltage) * 0.5; // Based on DFROBOT formula
  return tdsValue;
}
