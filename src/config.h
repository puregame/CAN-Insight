// general debug flag
// #define DEBUG
    // slows down startup to allow serial to open
    // prints on usb serial every can message received

/// ******  log file parameters *****
//char log_file_name[40] = "CAN_LOG_XXXX-XX-XXTHH:MM:SSZ.csv";
// #define LOG_FILE_DOT_POS 28
#define LOG_FILE_NAME_LENGTH 12
#define DEFAULT_LOG_FILE_NAME "CAN_000.log"
#define LOG_FILE_NUM_POS 4 // position of start of 3-set sequential numer of log
#define LOG_FILE_DOT_POS 7
#define DEFAULT_MAX_LOG_FILE_SIZE 1000000000 // 1 million bytes = 1 gigabyte

/// ******  config file parameters *****
#define CONFIG_FILE_NAME "config.txt"
// #define CONFIG_FILE_NAME "config.json"
#define UNIT_INFO_MAX_LEN 11

#define HEADER_CSV "timestamp,CAN_BUS,CAN_EXT,CAN_ID,CAN_LEN,Data0,Data1,Data2,Data3,Data4,Data5,Data6,Data7"

/// ******  can parameters *****

#define DEFAULT_BAUD_RATE 250000