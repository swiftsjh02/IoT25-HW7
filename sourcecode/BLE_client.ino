
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <math.h>

#define TARGET_NAME "DHT11_ESP32"
#define LED_PIN 2  // Built-in LED pin 

BLEScan* pBLEScan;
BLEAdvertisedDevice* targetDevice = nullptr;
bool found = false;

// Estimate distance from RSSI
float estimateDistance(int rssi, int txPower = -59, float n = 2.5) {
  return pow(10.0, ((float)(txPower - rssi)) / (10.0 * n));
}

// BLE Callback
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.haveName() && advertisedDevice.getName() == TARGET_NAME) {
      Serial.println("Target BLE device found!");
      targetDevice = new BLEAdvertisedDevice(advertisedDevice);
      found = true;
    }
  }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); // Initialize LED pin
  digitalWrite(LED_PIN, LOW); // Turn off LED initially

  Serial.println("Starting BLE scan...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(), true);
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(0, nullptr, true); // Continuous scan
}

void loop() {
  if (targetDevice != nullptr) {
    int rssi = targetDevice->getRSSI();
    float distance = estimateDistance(rssi);

    Serial.print("[" + targetDevice->getName() + "] RSSI: ");
    Serial.print(rssi);
    Serial.print(" dBm, Estimated Distance: ");
    Serial.print(distance, 2);
    Serial.println(" m");

    // LED Control
    if (distance <= 0.2) {
      digitalWrite(LED_PIN, HIGH);  // Turn on LED
    } else {
      digitalWrite(LED_PIN, LOW);   // Turn off LED
    }

  } else if (!found) {
    Serial.println("Searching for target device…");
  }

  delay(1000);
}
