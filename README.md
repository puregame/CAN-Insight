# Teensy 4.1 Based CAN message logger

This project is a Teensy 4.1 based CAN logger designed to log up to 3 CAN channels at configurable baudrates and with configurable filters.

Config file: the device is dynamically configurable by use of a JSON config file placed at the top-level directory of the SD card. See `CONFIG.TXT` for an example. If the config file does not exist some "reasonable" default values will be used. Any value not found in the config file will be defaulted to these values as well.

Default values:
- Baud rate 250k
- log both standard and extended frames with no filtering
- max file size 1GB
- start logging automatically on device bootup


NOTE: must remove `Time.h` from `C:\Program Files (x86)\Arduino\hardware\teensy\avr\libraries\Time` and must rename `Time.cpp` to `TimeLib.cpp`.

