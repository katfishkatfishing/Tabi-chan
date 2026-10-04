#include <arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEUtils.h>
#include <TinyGPSPlus.h>

#define SERVICE_UUID ""
#define CHARACTERISTIC_UUID ""

static const int RX_PIN = 16; // RX pin for GPS module
static const int TX_PIN = 17; // TX pin for GPS module
static const uint32_t GPS_BAUD = 9600; // Baud rate for GPS module

#pragma pack(push, 1)
struct GPSData {
    int32_t latitude;  // Latitude in microdegrees
    int32_t longitude; // Longitude in microdegrees
    int_16_t altitude;   // Altitude in centimeters
    uint16_t speed;      // Speed in centimeters per second
};
#pragma pack(pop)

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // Use hardware serial port 2 for GPS

BLEServer* pServer = nullptr; // BLE server instance
BLECharacteristic* pCharacteristic = nullptr; //BLE characteristic instance
bool deviceConnected = false; 
bool oldDeviceConnected = false; // Track the previous connection state

unsigned long lastUpdatetime = 0; //Timestamp of the previous update
const unsigned long UPDATE_INTERVAL_MS = 1000; 

//server connection callback
class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override { 
        deviceConnected = true;
        Serial.println("Tabi-chan has joined the party!");
    }

    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("Tabi-chan is leaving the party!");
    }
};

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);

    Serial.println("\n[Init] Waking up Tabi-chan...");

    BLEDevice::init("Tabi-chan");

    // Create BLE server and set callbacks
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    //create service
    BLEService* pService = pServer ->createService(SERVICE_UUID);

    //create characteristic
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );

    pCharacteristic->addDescriptor(new BLE2902());

    //start service and advertising
    pService->start();

    BLEAdviertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMiPreferred(0x06); // Set the advertising type to connectable undirected
    pAdvertising->setMinPreferred(0x12); // Set the minimum preferred connection interval
    BLEDevice::startAdvertising();

    Serial.print("Tabi-chan is now advertising as a BLE device!");
}

void loop() {
    //feed GPS data to TinyGPS++ 
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    //handle reconnection advertising
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("You disconnected, Tabi-chan is now advertising again!");
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }

    unsigned long currentMillis = millis();
    if (deviceConnected && (currentMillis - lastUpdatetime >= UPDATE_INTERVAL_MS)) {
        lastUpdateTime = currentMillis();

        GpsData gpsData;

        if (gps.location.isValid()) {
            //raw coords
            gpsData.latitude = (int32_t)(gps.location.lat() * 1e7);
            gpsData.longitude = (int32_t)(gps.location.lng() * 1e7);
            gpsData.altitude = (int16_t)(gps.altitude.meters());

            gpsData.speed = (uint16_t)(gps.speed.kmph() * 100 / 3.6); // Convert km/h to cm/s

            Serial.printf("[GPS] Lat: %.6f, Lng: %.6f, Alt: %.2f m, Speed: %.2f km/h\n", gps.location.lat(), gps.location.lng(), gps.altitude.meters(), gps.speed.kmph());
           
        } else {
            // fixed zero indicates acquiring satellites
            gpsData.latitude = 0;
            gpsData.longitude = 0;
            gpsData.altitude = 0;
            gpsData.speed = 0;

            Serial.printf("[GPS] Tabi-chan is searching for satellites... (I can see: %d)\n", gps.satellites.value());
        }

        //Notify web app with binary byte array
        pCharacteristic->setValue((uint8_t*)&gpsData, sizeof(GpsData));
        pCharacteristic->notify();
    }
}