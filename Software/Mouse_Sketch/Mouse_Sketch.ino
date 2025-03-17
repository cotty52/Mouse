#include <bluefruit.h>
#include <SPI.h>

// BLE Service
BLEDis bledis;
BLEHidAdafruit blehid;

// Pin definitions
const int LEFT_PIN = 3;
const int RIGHT_PIN = 21;

// PMW3389 SPI pins - adjust these to match your wiring
const int CS_PIN = 18;     // Chip select
const int MOTION_PIN = 17; // Motion
const int SCK_PIN = 16;    // SCK
const int MOSI_PIN = 15;   // MOSI
const int MISO_PIN = 14;   // MISO


// PMW3389 registers
#define REG_Product_ID  0x00
#define REG_Motion  0x02
#define REG_Delta_X_L  0x03
#define REG_Delta_X_H  0x04
#define REG_Delta_Y_L  0x05
#define REG_Delta_Y_H  0x06

// Button states
bool leftButtonState = false;
bool rightButtonState = false;
bool lastLeftButtonState = false;
bool lastRightButtonState = false;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  
  Serial.println("Bluetooth Mouse with PMW3389");
  
  // Initialize button pins
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  
  // Initialize SPI and sensor pins
  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();
  
  // Initialize PMW3389
  initializePMW3389();
  
  // Initialize Bluefruit
  Bluefruit.begin();
  Bluefruit.setName("meese");
  Bluefruit.setTxPower(4);
  
  bledis.setManufacturer("OTTY co");
  bledis.setModel("model dees nuts");
  bledis.begin();
  // uuuh ask claude why twice??
  blehid.begin();
  startAdv();
}

void initializePMW3389() {
  // Power up reset
  digitalWrite(CS_PIN, HIGH);
  delay(1);
  digitalWrite(CS_PIN, LOW);
  delay(1);
  digitalWrite(CS_PIN, HIGH);
  delay(1);

  // Read Product ID
  uint8_t pid = readRegister(REG_Product_ID);
  Serial.print("Product ID: 0x");
  Serial.println(pid, HEX);
  
  // Additional initialization commands would go here
  // These would be specific to your sensor configuration
  
  delay(100);
}

uint8_t readRegister(uint8_t reg_addr) {
  digitalWrite(CS_PIN, LOW);
  
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));
  SPI.transfer(reg_addr & 0x7f);
  uint8_t data = SPI.transfer(0);
  SPI.endTransaction();
  
  digitalWrite(CS_PIN, HIGH);
  delayMicroseconds(1);
  
  return data;
}

void writeRegister(uint8_t reg_addr, uint8_t data) {
  digitalWrite(CS_PIN, LOW);
  
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));
  SPI.transfer(reg_addr | 0x80);
  SPI.transfer(data);
  SPI.endTransaction();
  
  digitalWrite(CS_PIN, HIGH);
  delayMicroseconds(1);
}

void readMotionData(int16_t *dx, int16_t *dy) {
  if (readRegister(REG_Motion) & 0x80) {
    *dx = (int16_t)(readRegister(REG_Delta_X_H) << 8 | readRegister(REG_Delta_X_L));
    *dy = (int16_t)(readRegister(REG_Delta_Y_H) << 8 | readRegister(REG_Delta_Y_L));
  } else {
    *dx = 0;
    *dy = 0;
  }
}

void startAdv(void) {
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_MOUSE);
  Bluefruit.Advertising.addService(blehid);
  Bluefruit.Advertising.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);
  Bluefruit.Advertising.setFastTimeout(30);
  Bluefruit.Advertising.start(0);
}

void loop() {
  if (Bluefruit.connected()) {
    // Read button states
    leftButtonState = !digitalRead(LEFT_PIN);
    rightButtonState = !digitalRead(RIGHT_PIN);
    
    // Read motion data
    int16_t deltaX, deltaY;
    readMotionData(&deltaX, &deltaY);
    
    // Scale the movement (adjust these values based on your preferences)
    int8_t scaledX = constrain(deltaX / 32, -127, 127);
    int8_t scaledY = constrain(deltaY / 32, -127, 127);
    
    // Prepare button report
    uint8_t buttons = 0;
    if (leftButtonState) buttons |= MOUSE_BUTTON_LEFT;
    if (rightButtonState) buttons |= MOUSE_BUTTON_RIGHT;
    
    // Send mouse report with movement and buttons
    blehid.mouseReport(buttons, scaledX, scaledY, 0, 0);
    
    // Debug output if there's movement or button changes
    if (deltaX != 0 || deltaY != 0 || 
        leftButtonState != lastLeftButtonState || 
        rightButtonState != lastRightButtonState) {
      Serial.print("X: ");
      Serial.print(deltaX);
      Serial.print(" Y: ");
      Serial.print(deltaY);
      Serial.print(" Left: ");
      Serial.print(leftButtonState);
      Serial.print(" Right: ");
      Serial.println(rightButtonState);
    }
    
    lastLeftButtonState = leftButtonState;
    lastRightButtonState = rightButtonState;
  }
  
  delay(1); // Small delay to prevent overwhelming the BLE stack
}