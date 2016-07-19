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
  
This code is beerware; if you see me (or any other Electronic Cats 
member) at the local, and you've found our code helpful, 
please buy us a round!

Distributed as-is; no warranty is given.
************************************************************/
// Import libraries (BLEPeripheral depends on SPI)
#include <SPI.h>
#include <BLEPeripheral.h>

// define pins (varies per shield/board)
#define BLE_REQ     10
#define BLE_RDY     2
#define BLE_RST     9

// LED and button pin
#define LED_PIN     3
#define BUTTON_PIN  4

// create peripheral instance, see pinouts above
BLEPeripheral            blePeripheral        = BLEPeripheral(BLE_REQ, BLE_RDY, BLE_RST);

// create service
BLEService               FwService           = BLEService("0000fef5000010008000-00805f9b34fb");
BLEService               CtrService           = BLEService("21c5046267cb63a35c4c82b5b9939aeb");
BLEService               CerService          = BLEService("bbe877095b894433ab7f8b8eef0d8e37");
BLEService               BatService          = BLEService("0000180f00001000800000805f9b34fb");

// create FW_UPDATE_SERVICE characteristic
BLECharCharacteristic    FwUpdateRequest = BLECharCharacteristic("21c5046267cb63a35c4c82b5b9939aef", BLERead | BLEWrite);
BLECharCharacteristic    FwVersion = BLECharCharacteristic("21c5046267cb63a35c4c82b5b9939af0", BLERead | BLEWrite);

// create DEVICE_CONTROL_SERVICE characteristic
BLECharCharacteristic    LedVibrateCtrl  = BLECharCharacteristic("21c5046267cb63a35c4c82b5b9939aec", BLERead | BLEWrite);
BLECharCharacteristic    ButtonNotif = BLECharCharacteristic("21c5046267cb63a35c4c82b5b9939aed", BLERead | BLENotify);

// create CERTIFICATE_SERVICE characteristic
BLECharCharacteristic    SfidaCommands = BLECharCharacteristic("bbe877095b894433ab7f8b8eef0d8e39", BLERead | BLEWrite);
BLECharCharacteristic    CentralToSfida = BLECharCharacteristic("bbe877095b894433ab7f8b8eef0d8e38", BLERead | BLEWrite);
BLECharCharacteristic    SfidaToCentral = BLECharCharacteristic("bbe877095b894433ab7f8b8eef0d8e3ac", BLERead | BLEWrite);

// create BATTERY_SERVICE characteristic
BLECharCharacteristic    BatLevel = BLECharCharacteristic("00002a1900001000800000805f9b34fb", BLERead | BLEWrite);

void setup() {
  Serial.begin(115200);
#if defined (__AVR_ATmega32U4__)
  delay(5000);  //5 seconds delay for enabling to see the start up comments on the serial board
#endif

  // set LED pin to output mode, button pin to input mode
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  // set advertised local name and service UUID
  blePeripheral.setLocalName("EBISU"); // or Pokemon GO Plus
   blePeripheral.setDeviceName("EBISU");
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

  // begin initialization
  blePeripheral.begin();

  Serial.println(F("BLE LED Switch Peripheral"));
}

void loop() {
  // poll peripheral

  BLECentral central = blePeripheral.central();

  if (central) {
    // central connected to peripheral
    Serial.print(F("Connected to central: "));
    Serial.println(central.address());

    while (central.connected()) {
      // central still connected to peripheral

    }

    // central disconnected
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
