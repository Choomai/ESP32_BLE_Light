# ESP32 BLE Light Sensor

TL;DR: Use LDR, ESP32, and BLE to create a light sensor that can send light intensity data to a mobile app.

## How to use

Wire it up like this:
```
3V3
 |
[LDR]
 |
 +------ GPIO_0
 |
[10 kOhm resistor]
 |
GND
```