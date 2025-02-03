#include <MIDIUSB.h>


const int NUM_BUTTONS = 12;
const int buttonPins[NUM_BUTTONS] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
const int NUM_POTS = 5;
const int potPins[NUM_POTS] = {A0, A1, A2, A3, A4};
const byte CC_NUMBERS[NUM_POTS] = {7, 8, 9, 10, 11};

const byte patchValues[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

const byte BUTTON_11_CC = 82;
const byte BUTTON_12_CC = 83;
const byte BUTTON_13_CC = 120;
byte lastCCValue = 0;

bool lastButtonStates[NUM_BUTTONS];
unsigned long lastDebounceTimes[NUM_BUTTONS];
const int debounceDelay = 50;

float smoothedPotValues[NUM_POTS] = {0, 0, 0, 0, 0};
int lastPotValues[NUM_POTS] = {-1, -1, -1, -1, -1};

const float SMOOTHING_FACTOR = 0.1;

void setup() {
    for (int i = 0; i < NUM_BUTTONS; i++) {
        pinMode(buttonPins[i], INPUT_PULLUP);
        lastButtonStates[i] = HIGH;
        lastDebounceTimes[i] = 0;
    }
    
    for (int i = 0; i < NUM_POTS; i++) {
        pinMode(potPins[i], INPUT);
    }
    
    Serial.begin(115200);
}

int logScaleMap(int value) {
    float scaled = log10(value + 1) / log10(1024) * 127;
    return constrain(round(scaled), 0, 127);
}

void loop() {
    for (int i = 0; i < NUM_POTS; i++) {
        int rawValue = analogRead(potPins[i]);
        int midiValue = logScaleMap(rawValue);
        
        smoothedPotValues[i] = (SMOOTHING_FACTOR * midiValue) + ((1 - SMOOTHING_FACTOR) * smoothedPotValues[i]);
        int finalValue = round(smoothedPotValues[i]);
        
        if (finalValue != lastPotValues[i]) {
            sendControlChange(CC_NUMBERS[i], finalValue);
            Serial.print("Pot ");
            Serial.print(i);
            Serial.print(" CC: ");
            Serial.print(CC_NUMBERS[i]);
            Serial.print(" Value: ");
            Serial.println(finalValue);
            lastPotValues[i] = finalValue;
        }
    }

    for (int i = 0; i < NUM_BUTTONS; i++) {
        bool buttonState = digitalRead(buttonPins[i]);
        
        if (buttonState == LOW && lastButtonStates[i] == HIGH && 
            (millis() - lastDebounceTimes[i]) > debounceDelay) {
            
            if (i == NUM_BUTTONS - 1) {
                byte newValue = (lastCCValue == 0) ? 127 : 0;
                sendControlChange(BUTTON_13_CC, newValue);
                lastCCValue = newValue;
                Serial.print("Button 13 CC: ");
                Serial.print(BUTTON_13_CC);
                Serial.print(" Value: ");
                Serial.println(newValue);
            } 
            else if (i == NUM_BUTTONS - 2) {
                sendControlChange(BUTTON_12_CC, 127);
                Serial.print("Button 12 CC: ");
                Serial.println(BUTTON_12_CC);
            }
            else if (i == NUM_BUTTONS - 3) {
                sendControlChange(BUTTON_11_CC, 127);
                Serial.print("Button 11 CC: ");
                Serial.println(BUTTON_11_CC);
            }
            else {
                programChange(0, patchValues[i]);
                Serial.print("Button ");
                Serial.print(buttonPins[i]);
                Serial.print(" - Set ");
                Serial.print((i / 3) + 1);
                Serial.print(" Patch ");
                Serial.println((i % 3) + 1);
            }
            
            lastDebounceTimes[i] = millis();
        }
        lastButtonStates[i] = buttonState;
    }

    delay(10);
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