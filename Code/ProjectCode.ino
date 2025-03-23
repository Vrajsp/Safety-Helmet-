#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <DHT.h>

// LCD: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// SIM800L: TX, RX
SoftwareSerial sim800(2, 3);

// Components
const int gasPin = A0;           // MQ-9 analog pin
const int dhtPin = A1;           // DHT11 pin
const int buzzerPin = 6;         // Buzzer pin

// Thresholds
const int CH4_THRESHOLD = 705;   // Methane threshold in ppm
const int CO_THRESHOLD = 50;     // CO threshold in ppm
const int TEMP_THRESHOLD = 38;   // Temp threshold in °C

// DHT setup
#define DHTTYPE DHT11
DHT dht(dhtPin, DHTTYPE);

// Variables
int gasLevel;
float methanePPM = 0.0;
float coPPM = 0.0;
float temperature = 0.0;
float humidity = 0.0;

void setup() {
  lcd.begin(16, 2);
  sim800.begin(9600);
  dht.begin();
  Serial.begin(9600);

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  lcd.print("System Init...");
  Serial.println("System Initializing...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Read sensors
  gasLevel = analogRead(gasPin);
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  calculateGasLevels();

  // Serial Output
  Serial.print("Raw: "); Serial.println(gasLevel);
  Serial.print("CH4: "); Serial.print(methanePPM); Serial.println(" ppm");
  Serial.print("CO: "); Serial.print(coPPM); Serial.println(" ppm");
  Serial.print("Temp: "); Serial.print(temperature); Serial.print(" °C");
  Serial.print(" | Humidity: "); Serial.print(humidity); Serial.println(" %");
  Serial.println("----------------------------");

  // LCD: Gas
  lcd.setCursor(0, 0);
  lcd.print("CH4: "); lcd.print(methanePPM); lcd.print("ppm");
  lcd.setCursor(0, 1);
  lcd.print("CO: "); lcd.print(coPPM); lcd.print("ppm");
  delay(2000);

  // LCD: Temp & Humidity
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: "); lcd.print(temperature); lcd.print(" C");
  lcd.setCursor(0, 1);
  lcd.print("Humid: "); lcd.print(humidity); lcd.print(" %");
  delay(2000);

  // Threshold check
  if (methanePPM >= CH4_THRESHOLD || coPPM >= CO_THRESHOLD || temperature >= TEMP_THRESHOLD) {
    activateAlert();
  } else {
    deactivateAlert();
  }
}

void calculateGasLevels() {
  // Simulated mapping for ppm conversion
  methanePPM = map(gasLevel, 0, 1023, 0, 2000);
  coPPM = map(gasLevel, 0, 1023, 0, 100);
}

void activateAlert() {
  Serial.println("!!! ALERT TRIGGERED !!!");
  buzzerAlert();
  sendSMSAlert();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ALERT: High Gas!");
}

void deactivateAlert() {
  noTone(buzzerPin);
}

void buzzerAlert() {
  for (int i = 0; i < 3; i++) {
    tone(buzzerPin, 1000); // 1 kHz tone
    delay(1000);           // 1 sec ON
    noTone(buzzerPin);
    delay(500);            // 0.5 sec OFF
  }
}

void sendSMSAlert() {
  sim800.println("AT+CMGF=1");
  delay(500);
  sim800.println("AT+CMGS=\"+918149970263\""); // Change to your number
  delay(500);
  sim800.print("ALERT! High levels:\n");
  sim800.print("CH4: "); sim800.print(methanePPM); sim800.println(" ppm");
  sim800.print("CO: "); sim800.print(coPPM); sim800.println(" ppm");
  sim800.print("Temp: "); sim800.print(temperature); sim800.println(" C");
  sim800.write(26); // CTRL+Z to send
  delay(2000);
}
