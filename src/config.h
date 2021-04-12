// general debug flag
#define DEBUG
    // slows down startup to allow serial to open
    // prints on usb serial every can message received

/// ***** Time Parameters *****
#define TIME_FILE_NAME "NOW.TXT"
#define DEFAULT_TIME 1610331025 // Jan 10 2021 
#define TIME_HEADER  "T"   // Header tag for serial time sync message

/// ******  log file parameters *****
#define LOG_FILE_NAME_LENGTH 12
#define DEFAULT_LOG_FILE_NAME "CAN_000.log"
#define LOG_FILE_NUM_POS 4 // position of start of 3-set sequential numer of log
#define LOG_FILE_DOT_POS 7
#define DEFAULT_MAX_LOG_FILE_SIZE 1000000000 // 1 million bytes = 1 gigabyte

/// ******  config file parameters *****
#define CONFIG_FILE_NAME "config.txt"
// #define CONFIG_FILE_NAME "config.json"
#define UNIT_INFO_MAX_LEN 11
#define CONFIG_FILE_JSON_SIZE_BYTES 2000

#define HEADER_CSV "timestamp,CAN_BUS,CAN_EXT,CAN_ID,CAN_LEN,Data0,Data1,Data2,Data3,Data4,Data5,Data6,Data7"

/// ******  can parameters *****
#define DEFAULT_BAUD_RATE 250


// *** wifi parameters ****
#define WIFI_CS_PIN 10
#define WIFI_IRQ_PIN 14
#define WIFI_RESET_PIN 9
#define WIFI_EN_PIN 32

#define MAX_SAVED_NETWORK_COUNT 5
#define MAX_SERVER_LEN 60

#define DEFAULT_NO_NETWORK_RSSI_VALUE 99.99

#define SERVER_MAX_LEN 30



// EEPROM setups
#define EEPROM_LOGs_UPLOADED_LOCATION 10
