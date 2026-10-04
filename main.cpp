#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <TinyGPSPlus.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeSansBold9pt7b.h>

#define SERVICE_UUID ""
#define CHARACTERISTIC_UUID ""

static const int RX_PIN = 32; // RX pin for GPS module
static const int TX_PIN = 33; // TX pin for GPS module
static const uint32_t GPS_BAUD = 9600; // Baud rate for GPS module

#define EPD_CS 5
#define EPD_DC 17
#define EPD_RST 16
#define EPD_BUSY 4

//display driver (idk yet whatever is chosen)
//adjust driver class based on the display used
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(
    GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY)
);

const unsigned char epd_tabi_chan[] PROGMEM = {

};

#pragma pack(push, 1)
struct GPSData {
    int32_t latitude;  // Latitude in microdegrees
    int32_t longitude; // Longitude in microdegrees
    int16_t altitude;   // Altitude in centimeters
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

unsigned long lastEpdUpdateTime = 0; // Timestamp of the previous e-paper update
const unsigned long EPD_UPDATE_INTERVAL_MS = 3000; // Update e-paper every 60 seconds
uint16_t epdPartialUpdateCount = 0; // Count of partial updates performed
const uint16_t EPD_FULL_UPDATE_CYCLE = 60;

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

init readBatteryPercent() {
    uint32_t raw = analogRead(BATTERY_ADC_PIN);

    float measuredVoltage = (raw / 4095.0f) * 3.3f * 2.0f;

    int pct = (int)(((measuredVoltage - 3.2f) / (4.2f - 3.2f)) * 100.0f);
    return constrain(pct, 0, 100);
}

void drawBatteryIcon(int x, int y, int percentage) {
    display.drawRect(x, y, 28, 14, GxEPD_BLACK);
    display.fillRect(x + 28, y + 3, 3, 8, GxEPD_BLACK);

    int fillWidth = (percentage * 22) / 100;
    if (fillWidth > 0) {
        display.fillRect(x + 3, y + 3, fillWidth, 8, GxEPD_BLACK);
    }
}

void drawStaticLayout() {

    int batteryPct = readBatteryPercent();
    bool hasFix = gps.location.isValid();
    int sats = gps.satellites.value();

    if (fullRefresh) {
        display.setFullWindow();
        display.firstPage();
        do {
            display.fillScreen(GxEPD_WHITE);
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeSansBold9pt7b);

            display.setCursor(6, 18);
            display.print("Tabi-chan is here!");
            drawBatteryIcon(display.width() - 36, 6, batteryPct);

            display.drawFastHLine(0, 24, display.width(), GxEPD_BLACK);

            int imgX = (display.wdth() - 48) / 2;
            int imgY = 32;
            display.drawBitmap(imgX, imgY, epd_tabi_chan, 48, 48, GxEPD_BLACK);

            display.drawFastHLine(0, 88, display.width(), GxEPD_BLACK);

            display.setCursor(6, 108);
            if (hasFix) {
                display.printf("Tabi-system: Fix (%dS) | %s", sats, deviceConnected ? "Connected" : "Wait");
            } else {
                display.printf("Tabi-system: Search (%dS) | %s", sats, deviceConnected ? "Connected" : "Wait");
            }
        } while (display.nextPage());
    } else {
        display.setPartialWindow(0, 0, display.width(), display.height());
        display.firstPage();
        do {
            display.setTextColor(GxEPD_BLACK);
            display.setFont(&FreeSansBold9pt7b);

            display.fillRect(display.width() - 40, 4, 38, 18, GxEPD_WHITE);
            drawbatteryIcon(display.width() - 36, 6, batteryPct);

            display.fillRect(0, 92, display.width(),  30, GxEPD_WHITE);
            display.setCursor(6, 108);
            if (hasFix) {
                display.printf("Tabi-system: Fix (%dS) | %s", sats, deviceConnected ? "Connected" : "Wait");
            } else {
                display.printf("Tabi-system: Search (%dS) | %s", sats, deviceConnected ? "Connected" : "Wait");
            }
        } while (display.nextPage());
    }
}


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