# Teensy 4.1 Based CAN message logger

This project is a Teensy 4.1 based CAN logger designed to log up to 3 CAN channels at configurable baudrates and with configurable CAN filters.

## Features:
- 3 CANBus channels.
- CAN baudrates of 100, 250, 500, 1k.
- RTC. Log files and entries are time-stamped
- Up to 256GB SD card? -- Confirm this
- project and unit identifiers in log file

## Config file:
The device is dynamically configurable by use of a JSON config file placed at the top-level directory of the SD card named `CONFIG.TXT`. If the config file does not exist some "reasonable" default values will be used. If any config value does not exist in the config file it will be defaulted to these values as well.

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
As soon as boot up is complete the device will begin logging CAN data. A future feature is to manually start logging via a button press or specific CAN message.

## Filtering CAN messages to log
?need more research here, also need to actually implement it?

## Setting RTC
?add command for setting RTC in linux and windows?

## Output File
The code outputs files names CAN_XXX.log where XXX is a sequential number starting at 0. After 999 logs the file name convention will change to CANXXXX.log. After 9999 logs, if the parameter overwrite_logs is true log 000 will be overwritten, otherwise no further logging will occur.

Each log file is a CSV file, the first line is a JSON text with the project identifier, unit identifier, and CANBus configuration.

## Viewing Data
