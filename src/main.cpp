#include <Arduino.h>
#include <BLEDevice.h>
#include <BLESecurity.h>

constexpr uint8_t SensorPin = 0;
constexpr uint8_t ConnectedPin = 10;
constexpr uint8_t ReadPin = 20;
const std::string DeviceName = "Light Sensor";
const BLEUUID ServiceUuid = BLEUUID((uint16_t)0x181A);
const BLEUUID CharacteristicUuid = BLEUUID((uint16_t)0x2AFB);

BLEServer* pServer = nullptr;
BLECharacteristic *pCharacteristic = nullptr;

bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) override {
        deviceConnected = true;
        digitalWrite(ConnectedPin, HIGH);
        Serial.println("Device connected.");
    };

    void onDisconnect(BLEServer *pServer) override {
        deviceConnected = false;
        digitalWrite(ConnectedPin, LOW);
        Serial.println("Device disconnected.");
        pServer->startAdvertising();
    }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pCharacteristic) override {
        digitalWrite(ReadPin, HIGH);

        const uint16_t rawValue = analogRead(SensorPin);
        const uint32_t millivolts = analogReadMilliVolts(SensorPin);
        
        Serial.printf("raw=%u, voltage=%lu mV\n", rawValue, millivolts);

        // Map voltage/ADC to estimated Lux (Example: simple linear scaling)
        // GATT specification 0x2AFB requires value in Lux * 100
        float estimatedLux = (rawValue / 4095.0f) * 1000.0f; 
        uint32_t scaledLux = (uint32_t)(estimatedLux * 100.0f); // Lux with 0.01 resolution

        // Pack 24-bit value into 3 bytes (Little-Endian)
        uint8_t payload[3];
        payload[0] = (scaledLux) & 0xFF;
        payload[1] = (scaledLux >> 8) & 0xFF;
        payload[2] = (scaledLux >> 16) & 0xFF;

        pCharacteristic->setValue(payload, 3);

        delay(300);
        digitalWrite(ReadPin, LOW);
    }
};

void setup() {
    Serial.begin(115200);
    analogReadResolution(12);
    pinMode(ConnectedPin, OUTPUT);
    pinMode(ReadPin, OUTPUT);
    
    BLEDevice::init(DeviceName);

    // Create BLE server
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    // Create BLE service
    BLEService *pService = pServer->createService(ServiceUuid);

    // Create BLE characteristic
    pCharacteristic = pService->createCharacteristic(
        CharacteristicUuid,
        BLECharacteristic::PROPERTY_READ
    );
    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

    BLESecurity bleSecurity;
    bleSecurity.setCapability(ESP_IO_CAP_NONE);
    bleSecurity.setAuthenticationMode(ESP_LE_AUTH_NO_BOND);

    pService->start();
    Serial.println("BLE service started.");

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(ServiceUuid);
    pAdvertising->setScanResponse(true);

    BLEDevice::startAdvertising();
    Serial.println("BLE advertising started.");
}

void loop() {
}