# Teensy 4.1 Based CAN message logger

This project is a Teensy 4.1 based CAN logger designed to log up to 3 CAN channels at configurable baudrates and with configurable CAN filters.

## Features:
- 3 CANBus channels.
- CAN baudrates of 100, 250, 500, 1k.
- RTC. Log files and entries are time-stamped
- Up to 256GB SD card? -- Confirm this
- project and unit identifiers in log file

## Config file:
The device is dynamically configurable by use of a JSON config file placed at the top-level directory of the SD card named `CONFIG.TXT`. If the config file does not exist some ~reasonable~ default values will be used. If any config value does not exist in the config file it will be defaulted to these values as well.

Config file parameters
- max_file_size - maximum file size before creating a new log file in bytes (default 1000000000, ~1GB)
- unit_type - string of unit type, also could be project type or model name. Must be less than 11 characters.
- unit_number - string of unique unit identifier, may be a serial number or other unique identifier. Must be less than 11 characters.
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

## Setting RTC
### On Linux
Connect the device to your computer via USB port and run: `date +T%s > /dev/ttyS4` (where 'ttys4' is the serial port of the teensy).
On windows 

### On Windows
Open a powershell and run the following:
- `[System.IO.Ports.SerialPort]::getportnames()` to list available COM ports
- `$port= new-Object System.IO.Ports.SerialPort COM4` to create a new port object to connect to
- `$port.open()` to open the port
- `$port.WriteLine("T"+[int32](New-TimeSpan -Start (Get-Date "01/01/1970") -End (Get-Date)).TotalSeconds)` to write the current time to the device
- `$port.ReadLine()` to read a single line (do this multiple times to get down to the latest lines)

### Via SD Card File
This method is not recommended since it is less accurate, the boot delay of the device could cause inaccuracies. However it is the easiest.
1. Choose a specific time that you will to power the unit on. (typically 120 seconds or so in the future)
2. Use an Epoch time converter: https://www.epochconverter.com/ to convert the power-on time to an epoch time.
3. Create a new file on the SD card called NOW.TXT
4. In the file put only the integer value of the desired epoch timestamp.
5. Insert SD card into device.
6. Power up device at the exact time, realtime clock will be set and NOW.txt file will be deleted.

## Output File
The code outputs files names CAN_XXX.log where XXX is a sequential number starting at 0. After 999 logs the file name convention will change to CANXXXX.log. After 9999 logs, if the parameter overwrite_logs is true log 000 will be overwritten, otherwise no further logging will occur.

Each log file is a CSV file, the first line is a JSON text with the project identifier, unit identifier, and CANBus configuration.

## Viewing Data
See the [CAN Insight Log Processor](https://github.com/puregame/CANInsight-processing) for software to translate CSV files into useable datafiles for graphins and advanced analysis.


# Roadmap
## Software
1. RTC set via terminal/computer.
2. SD Card Full Behaivor.
3. CAN bus filtering for individual CAN messages.
4. Start/Stop logging on specific message ID received.
5. Support for full exFat filenames (current limitation is 7.3 format) 
6. Wifi data connection and automatic upload to server.

## Hardware
1. Test and validate LEDs
2. Test and validate Wifi