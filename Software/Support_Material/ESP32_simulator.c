#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// BLE
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CELL_VOLTAGES_UUID  "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CURRENT_UUID        "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define TEMP_UUID           "beb5483e-36e1-4688-b7f5-ea07361b26aa"
#define STATUS_UUID         "beb5483e-36e1-4688-b7f5-ea07361b26ab"

// Simul constants 
#define BASE_CELL_VOLTAGE 3400   
#define VOLTAGE_VARIATION 150     
#define VOLTAGE_DRIFT 5         
#define CURRENT_BASE 2000       
#define CURRENT_VARIATION 3000    
#define TEMP_BASE 25              
#define TEMP_VARIATION 10         
#define TEMP_DRIFT 1              
#define MAX_CELL_VOLTAGE 3600    
#define MAX_DISCHARGE_CURRENT 10000 
#define MIN_TEMP -20           
#define MAX_TEMP 40              


int16_t cell_voltages[4] = {3400, 3410, 3395, 3405};  // mV
int16_t current_mA = 2000;         // mA
int16_t temperatures[2] = {25, 26}; // °C
bool system_fault = false;

// Simul state
float voltage_targets[4] = {3400, 3410, 3395, 3405};
float current_target = 2000;
float temp_targets[2] = {25, 26};
unsigned long last_update = 0;
unsigned long fault_trigger_time = 0;
bool fault_mode_active = false;

BLEServer* pServer = NULL;
BLECharacteristic* pCellVoltagesChar = NULL;
BLECharacteristic* pCurrentChar = NULL;
BLECharacteristic* pTempChar = NULL;
BLECharacteristic* pStatusChar = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// Prototypes
void setup_ble(void);
void simulate_battery_data(void);
void check_fault_conditions(void);
void update_ble_data(void);


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
        Serial.println("BLE Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
        Serial.println("BLE Client Disconnected");
    }
};

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 BMS Simulator Starting...");
    
    setup_ble();
    
    randomSeed(analogRead(0));
    
    Serial.println("Simulator ready");
}

// Main
void loop() {
    unsigned long current_time = millis();
    
    if (current_time - last_update >= 100) {
        last_update = current_time;
        
        simulate_battery_data();
        
        check_fault_conditions();
        
        if (deviceConnected) {
            update_ble_data();
        }
    }
    
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }
}


void setup_ble(void) {
    BLEDevice::init("ESP32_BMS");
    
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    pCellVoltagesChar = pService->createCharacteristic(
        CELL_VOLTAGES_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCellVoltagesChar->addDescriptor(new BLE2902());
    
    pCurrentChar = pService->createCharacteristic(
        CURRENT_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCurrentChar->addDescriptor(new BLE2902());
    
    pTempChar = pService->createCharacteristic(
        TEMP_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pTempChar->addDescriptor(new BLE2902());
    
    pStatusChar = pService->createCharacteristic(
        STATUS_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pStatusChar->addDescriptor(new BLE2902());
    
    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE Service started...");
}

// Simulation stuff
void simulate_battery_data(void) {
    if (!fault_mode_active && random(0, 600) == 0) {
        fault_mode_active = true;
        fault_trigger_time = millis();
        Serial.println("FAULT!!!!!!!!");
        
        int fault_type = random(0, 3);
        switch(fault_type) {
            case 0: 
                voltage_targets[random(0, 4)] = 3650;
                Serial.println("Simulating overvoltage fault");
                break;
            case 1:
                current_target = 12000;
                Serial.println("Simulating overcurrent fault");
                break;
            case 2: 
                temp_targets[random(0, 2)] = random(0, 2) ? 45 : -25;
                Serial.println("Simulating temperature fault");
                break;
        }
    }
    
    if (fault_mode_active && (millis() - fault_trigger_time > 5000)) {
        fault_mode_active = false;
        voltage_targets[0] = BASE_CELL_VOLTAGE;
        voltage_targets[1] = BASE_CELL_VOLTAGE + 10;
        voltage_targets[2] = BASE_CELL_VOLTAGE - 5;
        voltage_targets[3] = BASE_CELL_VOLTAGE + 5;
        current_target = CURRENT_BASE;
        temp_targets[0] = TEMP_BASE;
        temp_targets[1] = TEMP_BASE + 1;
        Serial.println("FAULT CLEARED!!!!!!");
    }
    
    for (int i = 0; i < 4; i++) {
        voltage_targets[i] += random(-VOLTAGE_DRIFT, VOLTAGE_DRIFT + 1);
        
        if (!fault_mode_active) {
            voltage_targets[i] = constrain(voltage_targets[i], 
                                          BASE_CELL_VOLTAGE - VOLTAGE_VARIATION, 
                                          BASE_CELL_VOLTAGE + VOLTAGE_VARIATION);
        }
        
        cell_voltages[i] = (cell_voltages[i] * 9 + (int16_t)voltage_targets[i]) / 10;
        
        cell_voltages[i] += random(-2, 3);
    }
    
    current_target += random(-50, 51);
    if (!fault_mode_active) {
        current_target = constrain(current_target, 
                                  CURRENT_BASE - CURRENT_VARIATION, 
                                  CURRENT_BASE + CURRENT_VARIATION);
    }
    current_mA = (current_mA * 8 + (int16_t)current_target) / 9;
    current_mA += random(-10, 11);
    
    for (int i = 0; i < 2; i++) {
        temp_targets[i] += random(-TEMP_DRIFT, TEMP_DRIFT + 1) * 0.1;
        
        if (!fault_mode_active) {
            temp_targets[i] = constrain(temp_targets[i], 
                                       TEMP_BASE - TEMP_VARIATION, 
                                       TEMP_BASE + TEMP_VARIATION);
        }
        
        temperatures[i] = (temperatures[i] * 9 + (int16_t)temp_targets[i]) / 10;
        
        if (random(0, 5) == 0) {
            temperatures[i] += random(-1, 2);
        }
    }
    
    Serial.printf("Cells: %4d %4d %4d %4d mV | Current: %5d mA | Temps: %3d %3d °C | Fault: %s\n",
                  cell_voltages[0], cell_voltages[1], cell_voltages[2], cell_voltages[3],
                  current_mA, temperatures[0], temperatures[1],
                  system_fault ? "YES" : "NO");
}

void check_fault_conditions(void) {
    system_fault = false;
    
    for (int i = 0; i < 4; i++) {
        if (cell_voltages[i] > MAX_CELL_VOLTAGE) {
            system_fault = true;
            break;
        }
    }
    
    if (current_mA > MAX_DISCHARGE_CURRENT) {
        system_fault = true;
    }
    
    if (temperatures[0] < MIN_TEMP || temperatures[0] > MAX_TEMP ||
        temperatures[1] < MIN_TEMP || temperatures[1] > MAX_TEMP) {
        system_fault = true;
    }
}


void update_ble_data(void) {
    uint8_t cell_data[8];
    for (int i = 0; i < 4; i++) {
        cell_data[i * 2] = (cell_voltages[i] >> 8) & 0xFF;
        cell_data[i * 2 + 1] = cell_voltages[i] & 0xFF;
    }
    pCellVoltagesChar->setValue(cell_data, 8);
    pCellVoltagesChar->notify();
    
    uint8_t current_data[2];
    current_data[0] = (current_mA >> 8) & 0xFF;
    current_data[1] = current_mA & 0xFF;
    pCurrentChar->setValue(current_data, 2);
    pCurrentChar->notify();
    
    uint8_t temp_data[4];
    for (int i = 0; i < 2; i++) {
        temp_data[i * 2] = (temperatures[i] >> 8) & 0xFF;
        temp_data[i * 2 + 1] = temperatures[i] & 0xFF;
    }
    pTempChar->setValue(temp_data, 4);
    pTempChar->notify();
    
    uint8_t status = system_fault ? 0x01 : 0x00;
    pStatusChar->setValue(&status, 1);
    pStatusChar->notify();
}