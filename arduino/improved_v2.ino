#include <MIDIUSB.h>

// Matrix Configuration
const int ROWS = 5;
const int COLS = 4;

const int ROW_PINS[ROWS] = {2, 3, 4, 5, 6};
const int COL_PINS[COLS] = {7, 8, 9, 10};

const int POWER_LED = 11;
const int ACTIVITY_LED = 12;
const int STATE_LED = 13;

// Button State Tracking
bool currentButtonStates[ROWS][COLS];
bool lastButtonStates[ROWS][COLS];
unsigned long lastDebounceTime[ROWS][COLS];
const int debounceDelay = 30;

// Tempo Management
unsigned long lastTapTime = 0;
unsigned long currentTempo = 500; // Default to 120 BPM (500ms between beats)
const unsigned long LONG_PRESS_DURATION = 1000; // 1 second for long press
const unsigned long TAP_TIMEOUT = 2000; // Reset tap tempo if no tap for 2 seconds
const unsigned long DEFAULT_TEMPO = 500; // Default tempo (120 BPM)
unsigned long buttonPressStartTime = 0;
bool isLongPress = false;

// LED Management
unsigned long lastLedFlash = 0;
bool ledState = false;

// Button 20 State
bool button20State = false;

struct ButtonMapping {
    byte messageType;  // 0 for Program Change, 1 for Control Change
    byte number;       // Program number or CC number
    byte value;        // Used for CC value, ignored for Program Change
};

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

void updateTempo(unsigned long tapTime) {
    if (lastTapTime > 0 && (tapTime - lastTapTime) < TAP_TIMEOUT) {
        currentTempo = tapTime - lastTapTime;
        
        // Convert tempo to BPM for MIDI Clock
        float bpm = 60000.0 / currentTempo;
        Serial.print("Tempo: ");
        Serial.print(bpm);
        Serial.println(" BPM");
        
        // Send MIDI Clock Tempo message
        sendTempoMessage(bpm);
    }
    lastTapTime = tapTime;
}

void sendTempoMessage(float bpm) {
    // Convert BPM to microseconds per quarter note
    unsigned long mpqn = (unsigned long)(60000000.0 / bpm);
    
    // MIDI Tempo message (0xFF 0x51 0x03 followed by tempo in microseconds)
    byte tempoBytes[3] = {
        (mpqn >> 16) & 0xFF,
        (mpqn >> 8) & 0xFF,
        mpqn & 0xFF
    };
    
    // Send as MIDI System Exclusive message
    midiEventPacket_t tempoMsg = {0x0F, 0xFF, 0x51, 0x03};
    MidiUSB.sendMIDI(tempoMsg);
    
    for (int i = 0; i < 3; i++) {
        midiEventPacket_t dataByte = {0x0F, tempoBytes[i], 0, 0};
        MidiUSB.sendMIDI(dataByte);
    }
    
    MidiUSB.flush();
}

void handleTempoLED() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastLedFlash >= currentTempo) {
        ledState = !ledState;
        digitalWrite(ACTIVITY_LED, ledState);
        lastLedFlash = currentTime;
    }
}

void scanMatrix() {
    for (int row = 0; row < ROWS; row++) {
        digitalWrite(ROW_PINS[row], LOW);
        delayMicroseconds(10);
        
        for (int col = 0; col < COLS; col++) {
            bool buttonPressed = !digitalRead(COL_PINS[col]);
            int buttonNum = getButtonNumber(row, col);
            
            // Special handling for button 16 (tap tempo)
            if (buttonNum == 16) {
                handleTempoButton(buttonPressed, row, col);
            } else if (buttonPressed != currentButtonStates[row][col]) {
                if ((millis() - lastDebounceTime[row][col]) > debounceDelay) {
                    currentButtonStates[row][col] = buttonPressed;
                    
                    if (buttonPressed && !lastButtonStates[row][col]) {
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

void handleTempoButton(bool buttonPressed, int row, int col) {
    unsigned long currentTime = millis();
    
    if (buttonPressed != currentButtonStates[row][col]) {
        if ((currentTime - lastDebounceTime[row][col]) > debounceDelay) {
            currentButtonStates[row][col] = buttonPressed;
            
            if (buttonPressed) {
                // Button just pressed
                buttonPressStartTime = currentTime;
                isLongPress = false;
            } else {
                // Button released
                if (!isLongPress) {
                    // Normal tap - update tempo
                    updateTempo(currentTime);
                }
            }
            
            lastButtonStates[row][col] = buttonPressed;
            lastDebounceTime[row][col] = currentTime;
        }
    } else if (buttonPressed && !isLongPress) {
        // Check for long press while button is held
        if ((currentTime - buttonPressStartTime) >= LONG_PRESS_DURATION) {
            isLongPress = true;
            currentTempo = DEFAULT_TEMPO;
            lastTapTime = 0;
            Serial.println("Tempo reset to default (120 BPM)");
            sendTempoMessage(120);
        }
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
    handleTempoLED();
    delay(1);
}