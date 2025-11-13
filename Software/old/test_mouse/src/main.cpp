// Required for Arduino functions like Serial.begin() and delay()
#include <Arduino.h>

// Required libraries for NimBLE BLE HID functionality
#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>
// #include <NimBLE2902.h> // REMOVED: This header is not part of NimBLE-Arduino
#include <NimBLEHIDDevice.h> // This is the key library for HID over NimBLE

// Define the name of your BLE device
#define DEVICE_NAME "ESP32S3_NimBLE_Mouse" // Changed name to reflect NimBLE usage

// Define the HID Report Descriptor for a mouse.
// This descriptor tells the host (e.g., your computer) what kind of device this is
// and what data it will send.
// In this case, it defines:
// - A mouse (0x01, 0x02)
// - 3 buttons (Left, Right, Middle)
// - X and Y axis movement (relative, 8-bit signed)
// - Vertical wheel (scroll) movement (relative, 8-bit signed)
const uint8_t HID_REPORT_DESCRIPTOR[] = {
  0x05, 0x01,        // Usage Page (Generic Desktop)
  0x09, 0x02,        // Usage (Mouse)
  0xA1, 0x01,        // Collection (Application)
  0x09, 0x01,        //   Usage (Pointer)
  0xA1, 0x00,        //   Collection (Physical)
  0x05, 0x09,        //     Usage Page (Buttons)
  0x19, 0x01,        //     Usage Minimum (Button 1)
  0x29, 0x03,        //     Usage Maximum (Button 3)
  0x15, 0x00,        //     Logical Minimum (0)
  0x25, 0x01,        //     Logical Maximum (1)
  0x95, 0x03,        //     Report Count (3 buttons)
  0x75, 0x01,        //     Report Size (1 bit per button)
  0x81, 0x02,        //     Input (Data, Variable, Absolute) - buttons
  0x95, 0x01,        //     Report Count (1 byte for padding)
  0x75, 0x05,        //     Report Size (5 bits for padding)
  0x81, 0x03,        //     Input (Constant) - padding
  0x05, 0x01,        //     Usage Page (Generic Desktop)
  0x09, 0x30,        //     Usage (X)
  0x09, 0x31,        //     Usage (Y)
  0x15, 0x81,        //     Logical Minimum (-127)
  0x25, 0x7F,        //     Logical Maximum (127)
  0x75, 0x08,        //     Report Size (8 bits)
  0x95, 0x02,        //     Report Count (2 axes: X, Y)
  0x81, 0x06,        //     Input (Data, Variable, Relative) - X, Y
  0x09, 0x38,        //     Usage (Wheel)
  0x15, 0x81,        //     Logical Minimum (-127)
  0x25, 0x7F,        //     Logical Maximum (127)
  0x75, 0x08,        //     Report Size (8 bits)
  0x95, 0x01,        //     Report Count (1 wheel)
  0x81, 0x06,        //     Input (Data, Variable, Relative) - Wheel
  0xC0,              //   End Collection
  0xC0               // End Collection
};

// Global variables for NimBLE services and device
NimBLEHIDDevice* hid;
NimBLECharacteristic* inputReportCharacteristic; // Input report characteristic for sending mouse data
NimBLEAdvertising* advertising;

// Flag to indicate if a client is connected
bool connected = false;

// Callback class for NimBLE server events
class MyServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer) {
    connected = true;
    Serial.println("Client connected!");
    // Stop advertising once connected to save power and prevent multiple connections
    advertising->stop();
    Serial.println("Advertising stopped on connect.");
  }

  void onDisconnect(NimBLEServer* pServer) {
    connected = false;
    Serial.println("Client disconnected! Restarting advertising...");
    // Restart advertising to allow new connections
    advertising->start();
    Serial.println("Advertising restarted on disconnect.");
  }
};

// Function to send a mouse report
// buttons: bitmask for buttons (1=left, 2=right, 4=middle)
// dx: change in X position (-127 to 127)
// dy: change in Y position (-127 to 127)
// scroll: change in scroll wheel position (-127 to 127)
void sendMouseReport(uint8_t buttons, int8_t dx, int8_t dy, int8_t scroll) {
  if (connected) {
    uint8_t report[4];
    report[0] = buttons; // Button states
    report[1] = dx;      // X movement
    report[2] = dy;      // Y movement
    report[3] = scroll;  // Scroll movement
    inputReportCharacteristic->setValue(report, sizeof(report));
    inputReportCharacteristic->notify();
  }
}

// Setup function, runs once when the ESP32-S3 starts
void setup() {
  // Add a small delay to ensure Serial Monitor can catch early prints
  delay(100);
  Serial.begin(115200);
  Serial.println("\n--- Starting ESP32-S3 NimBLE HID Mouse ---"); // Updated print
  Serial.println("Initializing NimBLE Device...");

  NimBLEDevice::init(DEVICE_NAME);
  Serial.println("NimBLEDevice initialized.");

  Serial.println("Creating NimBLE Server...");
  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  Serial.println("NimBLE Server created with callbacks.");

  Serial.println("Creating NimBLE HID Device...");
  hid = new NimBLEHIDDevice(pServer);
  Serial.println("NimBLE HID Device created.");

  Serial.println("Setting Input Report Characteristic...");
  // Changed from inputReport to getInputReport based on compiler suggestion
  inputReportCharacteristic = hid->getInputReport(1); // Report ID 1 for mouse input
  Serial.println("Input Report Characteristic set.");

  Serial.println("Setting Manufacturer Data and Battery Level...");
  // Changed from setManufacturerData to setManufacturer based on compiler suggestion
  hid->setManufacturer("YourCompany"); // Replace with your company name
  // Removed hid->pnp(...) as it caused a compilation error and might not be directly exposed in this NimBLE version
  hid->setBatteryLevel(100); // Set initial battery level (optional)
  Serial.println("Manufacturer and Battery Level set.");

  Serial.println("Setting HID Report Map...");
  // setReportMap() is the correct method for NimBLE
  hid->setReportMap((uint8_t*)HID_REPORT_DESCRIPTOR, sizeof(HID_REPORT_DESCRIPTOR));
  Serial.println("HID Report Map set.");

  Serial.println("Starting HID Services...");
  hid->startServices();
  Serial.println("HID Services started.");

  Serial.println("Getting Advertising Object and configuring...");
  advertising = NimBLEDevice::getAdvertising();
  advertising->setAppearance(HID_MOUSE); // Set appearance to a mouse
  // Changed from hidService to getHidService based on compiler suggestion
  advertising->addServiceUUID(hid->getHidService()->getUUID());
  Serial.println("Advertising object configured.");

  Serial.println("Starting Advertising...");
  advertising->start();
  Serial.println("Advertising started!");
  Serial.println("BLE HID Mouse ready! Pair with your device.");
}

// Loop function, runs repeatedly
void loop() {
  if (connected) {
    // --- Simulate mouse movement in a square pattern ---
    Serial.println("Moving mouse right...");
    for (int i = 0; i < 5; i++) {
      sendMouseReport(0, 10, 0, 0); // Move right by 10
      delay(50);
    }
    delay(500);

    Serial.println("Moving mouse down...");
    for (int i = 0; i < 5; i++) {
      sendMouseReport(0, 0, 10, 0); // Move down by 10
      delay(50);
    }
    delay(500);

    Serial.println("Moving mouse left...");
    for (int i = 0; i < 5; i++) {
      sendMouseReport(0, -10, 0, 0); // Move left by 10
      delay(50);
    }
    delay(500);

    Serial.println("Moving mouse up...");
    for (int i = 0; i < 5; i++) {
      sendMouseReport(0, 0, -10, 0); // Move up by 10
      delay(50);
    }
    delay(500);

    // --- Simulate a left click ---
    Serial.println("Simulating left click...");
    sendMouseReport(0x01, 0, 0, 0); // Press left button (0x01)
    delay(100);
    sendMouseReport(0x00, 0, 0, 0); // Release all buttons (0x00)
    delay(1000);

    // --- Simulate a scroll down ---
    Serial.println("Simulating scroll down...");
    sendMouseReport(0, 0, 0, -5); // Scroll down by 5 units
    delay(100);
    sendMouseReport(0, 0, 0, 0); // Stop scrolling
    delay(1000);

    // --- Simulate a scroll up ---
    Serial.println("Simulating scroll up...");
    sendMouseReport(0, 0, 0, 5); // Scroll up by 5 units
    delay(100);
    sendMouseReport(0x00, 0, 0, 0); // Stop scrolling
    delay(1000);

  } else {
    // If not connected, just wait and print a message
    Serial.println("Waiting for connection...");
    delay(2000);
  }
}
