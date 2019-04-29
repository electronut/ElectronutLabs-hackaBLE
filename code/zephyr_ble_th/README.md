### Environmental Sensing Service Implementation with BME280 Sensor
This firmware implements enviromental sensing service on hackaBLE with BLE280. Temperature, humidity and pressure data can be accessed by enabling respective ble characteristic notifications.

#### Connections
| hackaBLE | BME280 |
|----------|--------|
| P0.03 | SCL |
| P0.04 | SDA|
| GND | GND |
| VDD | VDD |

#### Build and Flash
```
mkdir build && cd build
cmake -GNinja -DBOARD=nrf52_hackable ..
ninja # compile
ninja flash # flash
```

Check Logs:

Enable `CONFIG_UART_CONSOLE=y` in `prj.conf`

`minicom -b 115200 -D /dev/ttyACM1`