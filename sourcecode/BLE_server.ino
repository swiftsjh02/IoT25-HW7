#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>

// Uncomment the next line for Celsius (default)
// Comment it if you prefer Fahrenheit
#define temperatureCelsius

// BLE server name
#define bleServerName "DHT11_ESP32"

// DHT Sensor setup
#define DHTPIN 22       // GPIO pin where the DHT11 is connected
#define DHTTYPE DHT11  // DHT 11

DHT dht(DHTPIN, DHTTYPE);

float temp;
float tempF;
float hum;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

bool deviceConnected = false;

// UUIDs
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"

#ifdef temperatureCelsius
  BLECharacteristic dhtTemperatureCelsiusCharacteristics("cba1d466-344c-4be3-ab3f-189f80dd7518", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor dhtTemperatureCelsiusDescriptor(BLEUUID((uint16_t)0x2902));
#else
  BLECharacteristic dhtTemperatureFahrenheitCharacteristics("f78ebbff-c8b7-4107-93de-889a6a06d408", BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor dhtTemperatureFahrenheitDescriptor(BLEUUID((uint16_t)0x2902));
#endif

BLECharacteristic dhtHumidityCharacteristics("ca73b3ba-39f6-4ab3-91ae-186dc9577d99", BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor dhtHumidityDescriptor(BLEUUID((uint16_t)0x2903));

\
// BLE callbacks
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

void setup() {
  Serial.begin(115200);
  dht.begin();  // Initialize DHT sensor

  BLEDevice::init(bleServerName);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *dhtService = pServer->createService(SERVICE_UUID);

  // Temperature
  #ifdef temperatureCelsius
    dhtService->addCharacteristic(&dhtTemperatureCelsiusCharacteristics);
    dhtTemperatureCelsiusDescriptor.setValue("DHT temperature Celsius");
    dhtTemperatureCelsiusCharacteristics.addDescriptor(&dhtTemperatureCelsiusDescriptor);
  #else
    dhtService->addCharacteristic(&dhtTemperatureFahrenheitCharacteristics);
    dhtTemperatureFahrenheitDescriptor.setValue("DHT temperature Fahrenheit");
    dhtTemperatureFahrenheitCharacteristics.addDescriptor(&dhtTemperatureFahrenheitDescriptor);
  #endif  

  // Humidity
  dhtService->addCharacteristic(&dhtHumidityCharacteristics);
  dhtHumidityDescriptor.setValue("DHT humidity");
  dhtHumidityCharacteristics.addDescriptor(new BLE2902());
  
  dhtService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection...");
}

void loop() {
  if (deviceConnected) {
    if ((millis() - lastTime) > timerDelay) {
      // Read temperature and humidity
      temp = dht.readTemperature();
      tempF = dht.readTemperature(true); // true returns Fahrenheit
      hum = dht.readHumidity();

      // Check if any reading failed
      if (isnan(temp) || isnan(tempF) || isnan(hum)) {
        Serial.println("Failed to read from DHT sensor!");
        return;
      }

      #ifdef temperatureCelsius
        static char temperatureCTemp[6];
        dtostrf(temp, 6, 2, temperatureCTemp);
        dhtTemperatureCelsiusCharacteristics.setValue(temperatureCTemp);
        dhtTemperatureCelsiusCharacteristics.notify();
        Serial.print("Temperature Celsius: ");
        Serial.print(temp);
        Serial.print(" ºC");
      #else
        static char temperatureFTemp[6];
        dtostrf(tempF, 6, 2, temperatureFTemp);
        dhtTemperatureFahrenheitCharacteristics.setValue(temperatureFTemp);
        dhtTemperatureFahrenheitCharacteristics.notify();
        Serial.print("Temperature Fahrenheit: ");
        Serial.print(tempF);
        Serial.print(" ºF");
      #endif

      static char humidityTemp[6];
      dtostrf(hum, 6, 2, humidityTemp);
      dhtHumidityCharacteristics.setValue(humidityTemp);
      dhtHumidityCharacteristics.notify();
      Serial.print(" - Humidity: ");
      Serial.print(hum);
      Serial.println(" %");

      lastTime = millis();
    }
  }
}
