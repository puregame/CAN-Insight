// general debug flag
#define DEBUG
    // slows down startup to allow serial to open



/// ******  log file parameters *****
//char log_file_name[40] = "CAN_LOG_XXXX-XX-XXTHH:MM:SSZ.csv";
// #define LOG_FILE_DOT_POS 28
#define LOG_FILE_NAME_LENGTH 12
#define DEFAULT_LOG_FILE_NAME "CAN_000.log"
#define LOG_FILE_NUM_POS 4 // position of start of 3-set sequential numer of log
#define LOG_FILE_DOT_POS 7


#define HEADER_CSV "datetime, CAN_ID, Data0, Data1, Data2, Data3, Data4, Data5, Data6, Data7"