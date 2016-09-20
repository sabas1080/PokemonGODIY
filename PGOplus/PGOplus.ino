/************************************************************
POGPlusDIY.ino
Pokemon GO Plus - Pokemon Go Plus with Arduino
Andrés Sabas @ Electronic Cats an The Inventor's House
Original Creation Date: Jal 19, 2016
https://hackaday.io/project/12680-pokemon-go-plus-diy

This project is intended to collectively create a DIY
version of the famous device Nintendo Pokemon Go Plus
with Arduino and BLE Breakout

Development environment specifics:
  IDE: Arduino 1.6.8
  Hardware Platform:
  - Arduino Micro (Arduino UNO low memory)
  - Bluefruit LE - Bluetooth Low Energy (BLE 4.0) - nRF8001 Breakout
  - Or nRF51822 with Arduino Core

This code is beerware; if you see me (or any other Electronic Cats
member) at the local, and you've found our code helpful,
please buy us a round!

Distributed as-is; no warranty is given.
************************************************************/
// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>

// define pins (varies per shield/board)
/*
#define BLE_REQ     12
#define BLE_RDY     2
#define BLE_RST     8
*/

// define pins for nRF5
#define BLE_REQ     -1
#define BLE_RDY     -1
#define BLE_RST     -1

// LED and button pin
#define LED_R     9
#define LED_G     10
#define LED_B     11
#define BUTTON_PIN  4

// Pin Battery
#define VBATPIN A1

// create peripheral instance, see pinouts above
BLEPeripheral            blePeripheral       = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);
BLEBondStore             bleBondStore;


// create service
BLEService               FwService           = BLEService("0000fef5000010008000-00805f9b34fb");
BLEService               CtrService          = BLEService("21c5046267cb63a35c4c82b5b9939aeb");
BLEService               CerService          = BLEService("bbe877095b894433ab7f8b8eef0d8e37");
BLEService               BatService          = BLEService("0000180f00001000800000805f9b34fb");

// create FW_UPDATE_SERVICE characteristic
BLECharCharacteristic    FwUpdateRequest = BLECharCharacteristic("21c5046267cb63a35c4c82b5b9939aef", BLERead | BLEWrite);
BLECharCharacteristic    FwVersion = BLECharCharacteristic("21c5046267cb63a35c4c82b5b9939af0", BLERead | BLEWrite);

// create DEVICE_CONTROL_SERVICE characteristic
BLECharCharacteristic    LedVibrateCtrl  = BLECharCharacteristic("21c5046267cb63a35c4c82b5b9939aec", BLERead | BLEWrite);
BLECharCharacteristic    ButtonNotif = BLECharCharacteristic("21c5046267cb63a35c4c82b5b9939aed", BLERead | BLENotify);

// create CERTIFICATE_SERVICE characteristic
BLECharCharacteristic    SfidaCommands = BLECharCharacteristic("bbe877095b894433ab7f8b8eef0d8e39", BLERead | BLEWrite | BLENotify);
BLECharCharacteristic    CentralToSfida = BLECharCharacteristic("bbe877095b894433ab7f8b8eef0d8e38", BLERead | BLEWrite);
BLECharCharacteristic    SfidaToCentral = BLECharCharacteristic("bbe877095b894433ab7f8b8eef0d8e3a", BLERead | BLEWrite);

// create BATTERY_SERVICE characteristic
BLECharCharacteristic    BatLevel = BLECharCharacteristic("00002a1900001000800000805f9b34fb", BLERead);

int oldBatteryLevel = 0;  // last battery level reading from analog input

void setup() {
  Serial.begin(115200);
#if defined (__AVR_ATmega32U4__)
  delay(5000);  //5 seconds delay for enabling to see the start up comments on the serial board
#endif
  
  // set LEDs RGB pines to output mode
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  
  //Set button pin to input mode
  pinMode(BUTTON_PIN, INPUT);

  // clears bond data on every boot
  bleBondStore.clearData();

  blePeripheral.setBondStore(bleBondStore);

  // set advertised local name and service UUID
  blePeripheral.setLocalName("Pokemon GO Plus"); // or Pokemon GO Plus
  blePeripheral.setDeviceName("Pokemon GO Plus");
  blePeripheral.setAdvertisedServiceUuid(FwService.uuid());
  blePeripheral.setAdvertisedServiceUuid(CtrService.uuid());
  blePeripheral.setAdvertisedServiceUuid(CerService.uuid());
  blePeripheral.setAdvertisedServiceUuid(BatService.uuid());

  // add service and characteristics FW_UPDATE_SERVICE
  blePeripheral.addAttribute(FwService);
  blePeripheral.addAttribute(FwUpdateRequest);
  blePeripheral.addAttribute(FwVersion);

  // add service and characteristics DEVICE_CONTROL_SERVICE
  blePeripheral.addAttribute(CtrService);
  blePeripheral.addAttribute(LedVibrateCtrl);
  blePeripheral.addAttribute(ButtonNotif);

  // add service and characteristics CERTIFICATE_SERVICE characteristic
  blePeripheral.addAttribute(CerService);
  blePeripheral.addAttribute(SfidaCommands);
  blePeripheral.addAttribute(CentralToSfida);
  blePeripheral.addAttribute(SfidaToCentral);

  // add service and characteristics  BATTERY_SERVICE characteristic
  blePeripheral.addAttribute(BatService);
  blePeripheral.addAttribute(BatLevel);

  
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  
  // begin initialization
  blePeripheral.begin();

  Serial.println(F("Pokemon Go Plus!"));
}

void loop() {
  
  blePeripheral.poll();

  updateBatteryLevel();
 
}

void blePeripheralConnectHandler(BLECentral& central) {
  Serial.print(F("Connected event, POG: "));
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  Serial.print(F("Disconnected event, POG: "));
  Serial.println(central.address());
}

void updateBatteryLevel() {
  /* Read the current voltage level on the A1 analog input pin.
     This is used here to simulate the charge level of a battery 3.7v.
     NOTE: Consider the voltage of your battery 
  */
  int battery = analogRead(VBATPIN);
  battery *= 2;    // we divided by 2, so multiply back
  battery *= 3.3;  // Multiply by 3.3V, our reference voltage
  int batteryLevel = map(battery, 0, 1023, 0, 100);
  
  if((abs(batteryLevel - oldBatteryLevel) >= 1)){ //if the battery level has change >= 1%
  //if (batteryLevel != oldBatteryLevel) {      // if the battery level has changed
    Serial.print("Battery Level % is now: "); // print it
    Serial.println(batteryLevel);
    BatLevel.setValue(batteryLevel);  // and update the battery level characteristic
    oldBatteryLevel = batteryLevel;           // save the level for next comparison
  }
}
