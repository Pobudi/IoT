#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer *pServer;
bool connected = false;

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) override {
        String value = pCharacteristic->getValue(); 
        Serial.print("Write request received, value: ");
        Serial.println((String)value.c_str());

        if((String)value.c_str() == "on"){
          digitalWrite(2, HIGH);
        }
        else if((String)value.c_str() == "off"){
          digitalWrite(2, LOW);
        }
        pCharacteristic->setValue("Projekt IoT");
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  pinMode(2, OUTPUT);

  setupBleServer();
}

void setupBleServer() {
  BLEDevice::init("ESP32_BLE_Server");
  pServer = BLEDevice::createServer();
  
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic =
    pService->createCharacteristic(CHARACTERISTIC_UUID, 
                                  BLECharacteristic::PROPERTY_READ | 
                                  BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setValue("Projekt IoT");

  pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // iPhone specific settings
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read/write to it!");
}

void loop() {
  Serial.println(pServer->getConnectedCount());
  if (pServer->getConnectedCount() > 0) {
    Serial.println("BLE device connected!");
    connected = true;
  } else {
    if (connected) {
      Serial.println("No BLE connections.");
      BLEDevice::startAdvertising();
      connected = false;
    }
  }
  delay(500);
}