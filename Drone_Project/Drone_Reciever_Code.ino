/*
  Project: Precision Agriculture Drone (Receiver)
  Source: Project Report 
  Hardware: Arduino Uno, SCD40, NRF24L01, SD Card Module
*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SensirionI2CScd4x.h>
#include <nRF24L01.h>
#include <RF24.h>

// NRF24L01 setup
RF24 radio(8, 9); // CE=D8, CSN=D9
const byte address[6] = "00001";

struct ControlData {
  int throttle;
  int yaw;
  int pitch;
  int roll;
};
ControlData receivedData;

SensirionI2CScd4x scd4x;
const int chipSelect = 4;
File dataFile;

// Motor Pins
const int motorPin1 = 3;
const int motorPin2 = 5;
const int motorPin3 = 6;
const int motorPin4 = 10;

void setup() {
  Serial.begin(115200);
  
  // Radio Init
  radio.begin();
  radio.openReadingPipe(1, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();

  // Sensor Init
  Wire.begin();
  scd4x.begin(Wire);
  scd4x.startPeriodicMeasurement();

  // SD Init
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  
  // CSV Header
  dataFile = SD.open("env_data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("CO2,Temperature,Humidity,Throttle,Yaw,Pitch,Roll");
    dataFile.close();
  }
  
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(motorPin3, OUTPUT);
  pinMode(motorPin4, OUTPUT);
}

void loop() {
  // 1. Receive Control Data
  if (radio.available()) {
    radio.read(&receivedData, sizeof(receivedData));
    
    // Mixing Algorithm
    int m1 = constrain(receivedData.throttle + receivedData.pitch + receivedData.roll, 0, 255);
    int m2 = constrain(receivedData.throttle + receivedData.pitch - receivedData.roll, 0, 255);
    int m3 = constrain(receivedData.throttle - receivedData.pitch + receivedData.yaw, 0, 255);
    int m4 = constrain(receivedData.throttle - receivedData.pitch - receivedData.yaw, 0, 255);

    analogWrite(motorPin1, m1);
    analogWrite(motorPin2, m2);
    analogWrite(motorPin3, m3);
    analogWrite(motorPin4, m4);
  }

  // 2. Read Sensor & Log Data
  uint16_t co2;
  float temperature, humidity;
  uint16_t error;
  error = scd4x.readMeasurement(co2, temperature, humidity);

  if (error == 0) {
    dataFile = SD.open("env_data.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.print(co2); dataFile.print(",");
      dataFile.print(temperature, 2); dataFile.print(",");
      dataFile.print(humidity, 2); dataFile.print(",");
      dataFile.print(receivedData.throttle); dataFile.print(",");
      dataFile.print(receivedData.yaw); dataFile.print(",");
      dataFile.print(receivedData.pitch); dataFile.print(",");
      dataFile.println(receivedData.roll);
      dataFile.close();
    }
    // Debug Print
    Serial.print("CO2: "); Serial.print(co2);
    Serial.print("\tTemp: "); Serial.println(temperature);
  }
  delay(500);
}