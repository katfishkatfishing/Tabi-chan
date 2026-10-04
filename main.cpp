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
    0xff, 0xff, 0xff, 0xfc, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x0f, 0xff, 0xff, 
	0xff, 0xff, 0xfe, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x01, 0xff, 0xff, 
	0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xff, 
	0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xff, 
	0xff, 0xff, 0xe0, 0x00, 0x00, 0x01, 0x3f, 0xff, 0xff, 0xff, 0xe4, 0x01, 0xda, 0x15, 0xbf, 0xff, 
	0xff, 0xff, 0xe6, 0x30, 0x48, 0x30, 0x3f, 0xff, 0xff, 0xff, 0xca, 0x22, 0x03, 0x00, 0x3f, 0xff, 
	0xff, 0xff, 0x88, 0x63, 0x03, 0x80, 0x1f, 0xff, 0xff, 0xfe, 0x00, 0x47, 0xc3, 0xc0, 0x1f, 0xff, 
	0xff, 0xff, 0x00, 0x0f, 0xc7, 0xc0, 0x1f, 0xff, 0xff, 0xff, 0xc2, 0x1f, 0xef, 0xc0, 0x1f, 0xff, 
	0xff, 0xff, 0x82, 0x1e, 0x7c, 0xc0, 0x1f, 0xff, 0xff, 0xff, 0x82, 0x1e, 0x7c, 0xc0, 0x3f, 0xff, 
	0xff, 0xff, 0x80, 0x1e, 0x7c, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xc0, 0x3f, 0xff, 
	0xff, 0xff, 0x80, 0x0f, 0xff, 0xc0, 0x3f, 0xff, 0xff, 0xff, 0xc0, 0x07, 0xff, 0x80, 0x3f, 0xff, 
	0xff, 0xff, 0xe0, 0x17, 0xbb, 0x80, 0x7f, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xc7, 0x00, 0x7f, 0xff, 
	0xff, 0xff, 0xf0, 0x03, 0xfe, 0x00, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x1b, 0xfc, 0x01, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0x80, 0xf8, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf8, 0x7f, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xfd, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xfe, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xf3, 0xfe, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe1, 0xfe, 0x07, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0x08, 0x1c, 0x31, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x06, 0x01, 0xc0, 0xf9, 0xff, 
	0xff, 0xff, 0xf8, 0x77, 0x87, 0x8c, 0xe0, 0x7f, 0xff, 0xff, 0xf8, 0xe1, 0xfe, 0x02, 0x00, 0x7f, 
	0xff, 0xff, 0xf1, 0xcc, 0x00, 0x00, 0x06, 0x7f, 0xff, 0xff, 0xf1, 0x90, 0xc6, 0x00, 0x8c, 0x7f, 
	0xff, 0xff, 0xf3, 0x28, 0xc6, 0x00, 0x90, 0x7f, 0xff, 0xff, 0xe3, 0x30, 0xc6, 0x00, 0x80, 0x7f, 
	0xff, 0xff, 0xc2, 0x6e, 0x46, 0x00, 0x83, 0x7f, 0xff, 0xff, 0xc6, 0x5f, 0x86, 0x00, 0x0e, 0x7f, 
	0xff, 0xff, 0xcc, 0xbf, 0x86, 0x01, 0xc8, 0x7f, 0xff, 0xff, 0x99, 0x7f, 0x86, 0x03, 0xe0, 0x7f, 
	0xff, 0xff, 0x9a, 0xff, 0x06, 0x03, 0xe0, 0x7f, 0xff, 0xff, 0x91, 0xf8, 0x06, 0x03, 0xf0, 0xff, 
	0xff, 0xff, 0x33, 0xf0, 0x86, 0x01, 0xf7, 0xff, 0xff, 0xff, 0x37, 0xe8, 0xc6, 0x01, 0xfb, 0xff, 
	0xff, 0xff, 0xaf, 0xd8, 0xc6, 0x62, 0xfb, 0xff, 0xff, 0xff, 0x8f, 0x90, 0xc6, 0x72, 0xfb, 0xff, 
	0xff, 0xff, 0xe7, 0x71, 0xc6, 0x71, 0x7b, 0xff, 0xff, 0xff, 0xf8, 0xf1, 0xc6, 0x31, 0x77, 0xff, 
	0xff, 0xff, 0xff, 0xe1, 0x87, 0x30, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xe3, 0x87, 0x38, 0x7f, 0xff, 
	0xff, 0xff, 0xff, 0xc3, 0x8f, 0x38, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x87, 0x8f, 0x3c, 0x7f, 0xff, 
	0xff, 0xff, 0xff, 0x07, 0x8f, 0x3c, 0x7f, 0xff, 0xff, 0xff, 0xff, 0x07, 0x0f, 0x1e, 0x3f, 0xff, 
	0xff, 0xff, 0xfe, 0x0f, 0x1f, 0x1e, 0x3f, 0xff, 0xff, 0xff, 0xfe, 0x0f, 0x1f, 0x9e, 0x3f, 0xff, 
	0xff, 0xff, 0xfc, 0x1f, 0x1f, 0x8f, 0x1f, 0xff, 0xff, 0xff, 0xfc, 0x1e, 0x3f, 0xcf, 0x1f, 0xff, 
	0xff, 0xff, 0xfc, 0x1e, 0x3f, 0xcf, 0x1f, 0xff, 0xff, 0xff, 0xfc, 0x3e, 0x3f, 0xcf, 0x9f, 0xff
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

    pinMode(BATTERY_ADC_PIN, INPUT);
    analogReadResolution(12); // Set ADC resolution to 12 bits (0-4095)

    display.init(115200, true, 2, false);
    display.setRotation(1); // Adjust rotation as needed
    renderScreen(true); // Full refresh on startup

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

    if (currentMillis - lastEpdUpdateTime >= EPD_UPDATE_INTERNAL_MS) {
        lastEpdUpdateTime = currentMilliis;
        epdPartialRefreshCount++;

        if (epdPartialRefreshCount >= EPD_FULL_REFRESH_CYCLE) {
            epdPartialRefreshCount = 0;
            renderScreen(true); // Full refresh
        } else {
            renderScreen(false); // Partial refresh
        }
    }
}