#include "BLEDevice.h"
#include <Wire.h>

#define bleServerName "iTAG            "

static BLEUUID batteryServiceUUID("0000180f-0000-1000-8000-00805f9b34fb");
static BLEUUID batteryLevelCharacteristicUUID("00002a19-0000-1000-8000-00805f9b34fb");

static BLEUUID alertUUID("00001802-0000-1000-8000-00805f9b34fb");
static BLEUUID alertCharacteristicUUID("00002a06-0000-1000-8000-00805f9b34fb");

static BLERemoteCharacteristic* batteryLevelCharacteristic;
static BLERemoteCharacteristic* alertCharacteristic;

static boolean doConnect = false;
static boolean connected = false;
static BLEAddress *pServerAddress;
 
BLEClient* pClient;

bool connectToServer(BLEAddress pAddress) {
  pClient = BLEDevice::createClient();
   if (!pClient->connect(pAddress)) {
       Serial.println("Failed to connect to server!");
       return false;
   }
   Serial.println(" - Connected to server");
 
   BLERemoteService* pRemoteService = pClient->getService(batteryServiceUUID);
   if (pRemoteService == nullptr) {
       Serial.print("Failed to find Battery Service UUID: ");
       Serial.println(batteryServiceUUID.toString().c_str());
       pClient->disconnect();
       return false;
   }
   Serial.println(" - Found Battery Service");

   batteryLevelCharacteristic = pRemoteService->getCharacteristic(batteryLevelCharacteristicUUID);
   if (batteryLevelCharacteristic == nullptr) {
       Serial.print("Failed to find Battery Level Characteristic UUID: ");
       Serial.println(batteryLevelCharacteristicUUID.toString().c_str());
       pClient->disconnect();
       return false;
   }
   Serial.println(" - Found Battery Level Characteristic");
   Serial.println(batteryLevelCharacteristic->canNotify());

   BLERemoteService* pRemoteServiceAlert = pClient->getService(alertUUID);
   if (pRemoteServiceAlert == nullptr) {
       Serial.print("Failed to find alert Service UUID: ");
       pClient->disconnect();
       return false;
   }
   Serial.println(" - Found alert Service");

   alertCharacteristic = pRemoteServiceAlert->getCharacteristic(alertCharacteristicUUID);
   if (alertCharacteristic == nullptr) {
       Serial.print("Failed to find alert Characteristic UUID: ");
       pClient->disconnect();
       return false;
   }
   Serial.println(" - Found alert Characteristic");

   connected = true;
   return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.println(advertisedDevice.getName() == bleServerName);
    if (advertisedDevice.getName() == bleServerName) { 
      Serial.println("tutaj");
      advertisedDevice.getScan()->stop(); 
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); 
      doConnect = true; 
      Serial.println("Device found. Connecting!");
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting Arduino BLE Client application...");

  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();

  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);
}

void loop() {
  
    if (doConnect) {
      if (connectToServer(*pServerAddress)) {
        Serial.println("We are now connected to the BLE Server.");
        doConnect = false;
      } else {
        Serial.println("Failed to connect to the BLE Server.");
      }
    }
    if (connected && pClient != nullptr && !pClient->isConnected()) {
        Serial.println("BLE connection lost. Attempting to reconnect...");
        connected = false;
        doConnect = true; 
    }

    if (connected) {
      if (batteryLevelCharacteristic->canRead()) {
        uint8_t batteryLevel = batteryLevelCharacteristic->readUInt8();
        Serial.print("Battery Level: ");
        Serial.print(batteryLevel);
        Serial.println("%");
      }
    }
    if (Serial.available() > 0) {  
      String command = Serial.readStringUntil('\n'); 

      command.trim(); 

      if (command == "ON") { 
        if (alertCharacteristic->canWrite()) {
          uint8_t alertCode = 0x01;  
          alertCharacteristic->writeValue(&alertCode, 1);
        }
      } else if (command == "OFF") {  
        if (alertCharacteristic->canWrite()) {
          uint8_t alertCodeOff = 0x00;  
          alertCharacteristic->writeValue(&alertCodeOff, 1);
        }
      } else {  
        Serial.print("Unknown command: ");
        Serial.println(command);
      }
    }
    delay(1000);  
}