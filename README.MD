# DIY MIDI Controller with Arduino

This project implements a versatile MIDI controller using Arduino with USB MIDI capabilities. The controller features 12 buttons and 5 potentiometers, offering a mix of program changes (PC) and control change messages (CC) for flexible MIDI control.

I use this with Mainstage and my own project, so a lot of the decisions are based on my own use. Please reach out if you have any questions or suggestions.

<i>I'm not an experienced electronics programmer, so I'm sure there are many ways to improve this code. Please feel free to fork and do so, and I'll be happy to merge it.</i>


## Features

The controller provides multiple MIDI control options:
- 9 program change buttons for quick patch selection
- 2 dedicated control change buttons (CC 82 and 83)
- 1 toggle button for All Sound Off (CC 120)
- 5 analog potentiometers for continuous control

### Button Configuration
- Buttons 1-9: Program Changes (0-8)
- Button 11: CC 82 (sends value 127)
- Button 12: CC 83 (sends value 127)
- Button 13: CC 120 toggle (alternates between 0 and 127)

### Potentiometer Configuration
The five potentiometers are mapped to the following control change messages:
- Pot 1: CC 7
- Pot 2: CC 8
- Pot 3: CC 9
- Pot 4: CC 10
- Pot 5: CC 11

![image](./assets/Midi%20Interface%20Circuit.png)

## Hardware Requirements

- Arduino board with USB MIDI capability (I used the Arduino Leonardo)
- 12 momentary push buttons
- 5 potentiometers (10k)
- USB cable for connecting to computer
- Project enclosure (<i>optional: I used a takeout box :D</i>)

## Pin Configuration

### Digital Pins (Buttons)
- Buttons 1-9: Pins 2-10
- Button 11: Pin 11 (CC 82)
- Button 12: Pin 12 (CC 83)
- Button 13: Pin 13 (CC 120 toggle)

### Analog Pins (Potentiometers)
- Pot 1: A0
- Pot 2: A1
- Pot 3: A2
- Pot 4: A3
- Pot 5: A4


### My configuration (I use this with Mainstage)
<table>
    <tr>
        <th>Arduino Pin</th>
        <th>Function</th>
    </tr>
    <tr>
        <td>2</td>
        <td>set 1 patch 1</td>
    </tr>
    <tr>
        <td>3</td>
        <td>set 1 patch 2</td>
    </tr>
    <tr>
        <td>4</td>
        <td>set 1 patch 3</td>
    </tr>
    <tr>
        <td>5</td>
        <td>set 2 patch 1</td>
    </tr>
    <tr>
        <td>6</td>
        <td>set 2 patch 2</td>
    </tr>
    <tr>
        <td>7</td>
        <td>set 2 patch 3</td>
    </tr>
    <tr>
        <td>8</td>
        <td>set 3 patch 1</td>
    </tr>
    <tr>
        <td>9</td>
        <td>set 3 patch 2</td>
    </tr>
    <tr>
        <td>10</td>
        <td>set 3 patch 3</td>
    </tr>
    <tr>
        <td>11</td>
        <td>control change (82)</td>
    </tr>
    <tr>
        <td>12</td>
        <td>control change (83)</td>
    </tr>
    <tr>
        <td>13</td>
        <td>control change (120)</td>
    </tr>
    <tr>
        <td>A0</td>
        <td>potentiometer 1 (cc 7)</td>
    </tr>
    <tr>
        <td>A1</td>
        <td>potentiometer 2 (cc 8)</td>
    </tr>
    <tr>
        <td>A2</td>
        <td>potentiometer 3 (cc 9)</td>
    </tr>
    <tr>
        <td>A3</td>
        <td>potentiometer 4 (cc 10)</td>
    </tr>
    <tr>
        <td>A4</td>
        <td>potentiometer 5 (cc 11)</td>
    </tr>
</table>
<i> To modify this, you can change the CC numbers in the code to match your own reqiurements as per the MIDI standard. </i>

## Software Dependencies

- MIDIUSB Library
  ```
  # In your Arduino IDE:
  Tools -> Manage Libraries -> Search for "MIDIUSB"
  ```

## Discussion

### Button Implementation
- All buttons use internal pull-up resistors
- Debouncing is implemented with a 50ms delay
- Program change buttons (1-9) send on channel 0
- CC buttons send standard control change messages
- The toggle button (13) maintains state between presses. I use this for instant mute.

### Potentiometer Features
- Logarithmic scaling for smoother response. (I noticed that it was too sensitive at higher values so I added a logarithmic scaling.)
- Exponential Moving Average (EMA) filtering to reduce noise
- Sends MIDI CC messages only when values change significantly

### Signal Processing
- Smoothing factor of 0.1 for potentiometer readings
- Built-in debouncing for all buttons
- Logarithmic scaling for more musical control of parameters

## Setup

1. Clone this repository.

2. Follow the schematics to connect the buttons and potentiometers to the Arduino.

3. Install the MIDIUSB library in Arduino IDE.

4. Select the sketch from the repo you have clined

5. Upload the code to your Arduino board.

6. Connect your Arduino to your computer via USB.

7. The device will appear as a USB MIDI device

8. Configure your DAW or MIDI software to receive messages from the controller

9. You're good to go!

## Customization

The code is designed to be easily customizable. Common modifications include:

- Change CC numbers by modifying the `CC_NUMBERS` array
- Adjust program change values in the `patchValues` array
- Modify the smoothing factor (`SMOOTHING_FACTOR`)
- Change debounce delay (`debounceDelay`)
- Adjust button behavior in the main loop

## Troubleshooting

Common issues and solutions:

1. No MIDI signals detected:
   - Check USB connection
   - Verify MIDI device recognition in your OS
   - Confirm MIDIUSB library installation

2. Erratic potentiometer behavior:
   - Adjust `SMOOTHING_FACTOR`
   - Check for noisy power supply
   - Verify potentiometer connections

3. Button bounce issues:
   - Increase `debounceDelay`
   - Check button connections
   - Verify pull-up resistor configuration

## Contributing

Contributions are welcome! Please feel free to create a fork and submit a pull request.

## License

This project is licensed under the MIT License - see the LICENSE.md file for details.