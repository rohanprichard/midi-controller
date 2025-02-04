#include <MIDIUSB.h>

// Matrix Configuration
const int ROWS = 5;
const int COLS = 4;

const int ROW_PINS[ROWS] = {2, 3, 4, 5, 6};
const int COL_PINS[COLS] = {7, 8, 9, 10};

const int POWER_LED = 11;      // Always on
const int ACTIVITY_LED = 12;   // Lights when button pressed
const int STATE_LED = 13;      // Mirrors button 20's state

// Button State Tracking
bool currentButtonStates[ROWS][COLS];
bool lastButtonStates[ROWS][COLS];
unsigned long lastDebounceTime[ROWS][COLS];
const int debounceDelay = 50;

unsigned long activityLedTimer = 0;
const int activityLedDuration = 100; 

bool button20State = false;

struct ButtonMapping {
    byte messageType;  // 0 for Program Change, 1 for Control Change
    byte number;       // Program number or CC number
    byte value;        // Used for CC value, ignored for Program Change
};

// Create a 5x4 mapping for each button in the matrix
ButtonMapping buttonMappings[ROWS][COLS] = {
    {{0, 0, 0}, {0, 1, 0}, {0, 2, 0}, {0, 3, 0}},
    {{0, 4, 0}, {0, 5, 0}, {0, 6, 0}, {0, 7, 0}},
    {{0, 8, 0}, {0, 9, 0}, {0, 10, 0}, {0, 11, 0}},
    {{1, 20, 127}, {1, 21, 127}, {1, 22, 127}, {1, 23, 127}},
    {{1, 24, 127}, {1, 25, 127}, {1, 26, 127}, {1, 120, 127}}
};

void setup() {
    for (int i = 0; i < ROWS; i++) {
        pinMode(ROW_PINS[i], OUTPUT);
        digitalWrite(ROW_PINS[i], HIGH);
    }
    
    for (int i = 0; i < COLS; i++) {
        pinMode(COL_PINS[i], INPUT_PULLUP);
    }
    
    pinMode(POWER_LED, OUTPUT);
    pinMode(ACTIVITY_LED, OUTPUT);
    pinMode(STATE_LED, OUTPUT);
    
    digitalWrite(POWER_LED, HIGH);
    
    Serial.begin(115200);
}

int getButtonNumber(int row, int col) {
    return (row * COLS) + col + 1;
}

void handleActivityLED() {
    if (activityLedTimer > 0 && millis() - activityLedTimer >= activityLedDuration) {
        digitalWrite(ACTIVITY_LED, LOW);
        activityLedTimer = 0;
    }
}

void scanMatrix() {
    for (int row = 0; row < ROWS; row++) {
        digitalWrite(ROW_PINS[row], LOW);
        delayMicroseconds(10);
        
        for (int col = 0; col < COLS; col++) {
            bool buttonPressed = !digitalRead(COL_PINS[col]);
            
            if (buttonPressed != currentButtonStates[row][col]) {
                if ((millis() - lastDebounceTime[row][col]) > debounceDelay) {
                    currentButtonStates[row][col] = buttonPressed;
                    
                    if (buttonPressed && !lastButtonStates[row][col]) {
                        digitalWrite(ACTIVITY_LED, HIGH);
                        activityLedTimer = millis();
                        
                        handleButtonPress(row, col);
                    }
                    
                    lastButtonStates[row][col] = buttonPressed;
                    lastDebounceTime[row][col] = millis();
                }
            }
        }
        
        digitalWrite(ROW_PINS[row], HIGH);
    }
}

void sendControlChange(byte control, byte value) {
    midiEventPacket_t event = {0x0B, 0xB0, control, value};
    MidiUSB.sendMIDI(event);
    MidiUSB.flush();
}

void programChange(byte channel, byte program) {
    midiEventPacket_t pc = {0x0C, 0xC0 | channel, program, 0};
    MidiUSB.sendMIDI(pc);
    MidiUSB.flush();
}

void handleButtonPress(int row, int col) {
    ButtonMapping mapping = buttonMappings[row][col];
    int buttonNum = getButtonNumber(row, col);
    
    Serial.print("Button ");
    Serial.print(buttonNum);
    Serial.print(" pressed (Row: ");
    Serial.print(row + 1);
    Serial.print(", Col: ");
    Serial.print(col + 1);
    Serial.print(") - ");
    
    if (buttonNum == 20) {
        button20State = !button20State;
        byte value = button20State ? 127 : 0;
        digitalWrite(STATE_LED, button20State ? HIGH : LOW);
        sendControlChange(mapping.number, value);
        Serial.print("Control Change: ");
        Serial.print(mapping.number);
        Serial.print(" Value: ");
        Serial.println(value);
    } else if (mapping.messageType == 0) {
        programChange(0, mapping.number);
        Serial.print("Program Change: ");
        Serial.println(mapping.number);
    } else {
        sendControlChange(mapping.number, mapping.value);
        Serial.print("Control Change: ");
        Serial.print(mapping.number);
        Serial.print(" Value: ");
        Serial.println(mapping.value);
    }
}

void loop() {
    scanMatrix();
    handleActivityLED();
    delay(1);
}

