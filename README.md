# CANInsight CAN logger

This project is a 3 CAN channel CAN logger designed from the ground up for ease of use.

## Why make this?
This project was born out of the desire for a highly customizable and inexpensive CAN logger. Other solutions on the market lack realtime clock (RTC) functionality, ony have a single channel, and/or are quite expensive. Furthermore the standard DB9 connector used on most CAN loggers are not dust or water proof and these devices are not suitable for installing on vehicles for long-term testing and data collection.

## Features:
- Up to 3 CANBus channels (default is 2).
- CAN baudrates of 125, 250, 500, 1k
- RTC. Log files and entries are time-stamped
- Logs CAN data to a micro-sd card
- Log files are tagged for specific projects/vehicle types and unit/serial/VIN (see below for details)
- LEDs for power and logging status
- 8-60v supported input voltages
- Reverse polarity protection
- Log processor to translate raw data into physical values
- CAN termination resistors with solder jumper
- IP44 rated case

## Device Configuration
The device is dynamically configurable by use of a JSON config file placed at the top-level directory of the SD card named `CONFIG.TXT`. If the config file does not exist some ~reasonable~ default values will be used. If any config value does not exist in the config file it will be defaulted to these values as well.

Config file parameters
- max_file_size - maximum file size before creating a new log file in bytes (default 1000000000, ~1GB)
- unit_type - string of unit type, also could be project type or model name. Must be less than 11 characters.
- unit_number - string of unique unit identifier, may be a serial number or other unique identifier. Must be less than 11 characters.
- log_type - string 3-char, options: "CSV", "DAT" "BLF"(future). DAT=slightly better data format
- overwrite_logs - boolean, if set to true then when log file list is full it will begin deleting and writing to log 000. (default false)
- can_1, can_2, can_3 - dictionary of values for each CAN network

    - baud_rate - CAN Bus baud rate in kbits/s (Default 250)
    - bus_name - name or description for the bus. Must be less than 21 characters.
    - log_extended_frames - boolean (true/false) (default true)
    - log_standard_frames - boolean (true/false) (default true)
    - id_filter_mask - integer filter mask, see filtering notes
    - id_filter_vale - integer filter value, see filtering notes

## Starting CAN logging
As soon as boot up is complete the device will begin logging CAN data. A feature on the roadmap is to manually start logging via a button press or specific CAN message.

## Filtering CAN messages to log
To be implemented, this is supported by the device and software but needs to be thought out and implemented. See the roadmap section for more details.

## Status LEDs
The CAN Insight device contains 2 status leds.

1. Green LED indicates power is on the unit.
2. RGB LED indicates program status.

#### RGB Status
Orange: System is booting up.
Red: SD Card not found or read error.
Green: Acquiring data.
Blue Flash: Writing data to SD Card.

## Setting RTC
### On Linux
Connect the device to your computer via USB port and run: `date +T%s > /dev/ttyS4` (where 'ttys4' is the serial port of the teensy).
On windows 

### On Windows
Open a powershell and run the following:
- `[System.IO.Ports.SerialPort]::getportnames()` to list available COM ports
- `$port= new-Object System.IO.Ports.SerialPort COM4` to create a new port object to connect to
- `$port.open()` to open the port
- `$port.WriteLine("T"+[int64](New-TimeSpan -Start (Get-Date "01/01/1970") -End (Get-Date).ToUniversalTime()).TotalSeconds)` to write the current time to the device
- `$port.ReadLine()` to read a single line (do this multiple times to get down to the latest lines)

### Via SD Card File
This method is not recommended since it is less accurate, the boot delay of the device could cause inaccuracies. However it is the easiest.
1. Choose a specific time that you will to power the unit on. (typically 120 seconds or so in the future)
2. Use an Epoch time converter: https://www.epochconverter.com/ to convert the power-on time to an epoch time.
3. Create a new file on the SD card called NOW.TXT
4. In the file put only the integer value of the desired epoch timestamp.
5. Insert SD card into device.
6. Power up device at the exact time, realtime clock will be set and NOW.txt file will be deleted.

## SD Card
The SD Card is required for data logging and configuration setting. If the SD card is not detected the Status LED will turn RED. If the status LED is RED the system will check for an SD card every second. As soon as a card is detected the time file will be checked, configuration will be loaded and logging will begin.

When the SD card is full logging will stop and no more data will be written to the SD Card. If the SD card is full and the device is restarted the status LED will remain orange and logging will not occur.

NOTE: SD Card MUST be formatted in FAT32 format!

### Input files
- now.txt - see setting time via SD method section for details
- config.txt - see config file section for details

### Output File
The code outputs files names CAN_XXX.log where XXX is a sequential number starting at 0. After 999 logs the file name convention will change to CANXXXX.log. After 9999 logs no further logging will occur.

Each log file is a CSV file, the first line is a JSON text with the project identifier, unit identifier, and CANBus configuration.

#### Compact Log File Format
Each line is of the format: "%2.2f-%d-%X#([%0.2X]*LEN)"
- Float representing time since start of logging
- Int representing CAN bus number
- Hex representing CAN message ID
- N* 2-char hex representing CAN data

## Viewing Data
See the [CAN Insight Log Processor](https://github.com/puregame/CANInsight-processing) for software to translate CSV files into useable datafiles for graphins and advanced analysis.


# Roadmap
## Software
1. CAN bus filtering for individual CAN messages.
2. Start/Stop logging on specific message ID received.
3. Support for full exFat filenames (current limitation is 7.3 format)

## Hardware
None

# Wifi
Putting general notes here for now, will clean up later.
1. No WEP supported in this project. 
2. If "wifi" tag is in config (it must have a pass?) if the password is an empty string the system will attempt to connect assuming the network is unencrypted.
3. Only connects to WPA or unencrypted wifi.