# DIY Arduino MIDI Controller

A USB-MIDI controller built around an Arduino Leonardo: twelve buttons select patches or send control changes, while five potentiometers provide continuous MIDI controls.

## Build assets

- [Circuit image](assets/Midi%20Interface%20Circuit.png)
- [Fritzing schematic](schematics/MIDI%20controller.fzz)
- Firmware: `arduino/main.ino`, `arduino/improved_v1.ino`, and `arduino/improved_v2.ino`

## Controls

| Control | MIDI behaviour |
| --- | --- |
| Buttons 1–9 | Program changes 0–8 |
| Button 11 | CC 82, value 127 |
| Button 12 | CC 83, value 127 |
| Button 13 | CC 120 toggle (all sound off) |
| Pots 1–5 | CC 7–11 |

The implementation includes pull-up button inputs, debounce handling, exponential moving-average smoothing, and value-change thresholds for the potentiometers.

## Parts

- Arduino board with native USB MIDI support (Arduino Leonardo used here)
- 12 momentary buttons
- 5 × 10k potentiometers
- USB data cable, wiring, and optional enclosure

## Build and upload

1. Install the **MIDIUSB** library in the Arduino IDE.
2. Wire the controller using the circuit image and Fritzing file.
3. Open the firmware version you want to use in the Arduino IDE.
4. Select the correct board and upload it.
5. Connect the board by USB; it should appear as a MIDI input device.
6. Map the messages in MainStage, a DAW, or other MIDI software.

## Customisation

Edit the CC numbers, program-change values, smoothing factor, or debounce delay in the sketch to match your own rig. Test each changed mapping before relying on it in a live setup.

## Troubleshooting

- No MIDI signal: check the USB data cable, native-MIDI board support, and MIDIUSB installation.
- Noisy controls: check wiring and adjust the smoothing factor.
- Double-triggered buttons: check connections and increase the debounce delay.

## License

MIT
