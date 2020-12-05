## Lumos
A Bluetooth Controlled LED Strip Project. It uses an ESP32 microcontroller to drive a 1M strip of 60 WS2813 LEDs. The project is controlled via Bluetooth using an Android app and supports multiple effects out of the box

Twinkle 
![alt](https://thumbs.gfycat.com/InnocentHighlevelInganue-small.gif)

Wave through a palette
![alt](https://thumbs.gfycat.com/CleanReasonableFrog-small.gif)

Fade through a palette
![alt](https://thumbs.gfycat.com/HappygoluckyHomelyApe-small.gif)

### Getting the parts
 - ESP32 Dev Kit
 - WS2813 Strip
 - Jumpers
 - Power Supply

### Connections
 - DATA pin to GPIO 19 
 - V <sub>in</sub> and GND to their respective pins

### Getting Started
 - compile and load `lumos.ino` onto the ESP32

### Note
Although the LED strip can be driven using power from the ESP32, it's not advisable to do so for long periods of time or for a large number (>60) of LEDs. I have tested this with a 1M strip having 60 LEDs which works fine but, it's good to have a separate power supply to drive both the ESP and the strip. At max brightness, each LED can draw about 60mA which results in a maximum current draw of 3.6A. This is way beyond what a microcontroller's terminals can handle.
