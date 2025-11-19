/*
  Project: Precision Agriculture Drone (Ground Station)
  Source: Project Report 
  Hardware: Arduino Uno, USB Host Shield, Xbox Controller, NRF24L01
*/

#include <XBOXUSB.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

USB Usb;
XBOXUSB Xbox(&Usb);

RF24 radio(8, 9); // CE=D8, CSN=D9
const byte address[6] = "00001";

struct ControlData {
  int throttle;
  int yaw;
  int pitch;
  int roll;
};
ControlData data;
int throttleLevel = 0;

void setup() {
  Serial.begin(115200);
  if (Usb.Init() == -1) {
    Serial.println(F("OSC did not start"));
    while (1);
  }
  
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  radio.stopListening();
}

void loop() {
  Usb.Task();
  if (Xbox.Xbox360Connected) {
    int rtValue = Xbox.getButtonPress(R2);
    int ltValue = Xbox.getButtonPress(L2);

    throttleLevel += rtValue / 10;
    throttleLevel -= ltValue / 10;
    throttleLevel = constrain(throttleLevel, 0, 255);
    data.throttle = throttleLevel;

    data.yaw = map(Xbox.getAnalogHat(LeftHatX), -32766, 32767, -128, 127) + 1;
    data.pitch = map(Xbox.getAnalogHat(RightHatY), -32766, 32767, -128, 127) + 1;
    data.roll = map(Xbox.getAnalogHat(RightHatX), -32766, 32767, -128, 127) + 1;

    radio.write(&data, sizeof(data));
    
    Serial.print("Throttle: "); Serial.println(data.throttle);
  }
  delay(10);
}