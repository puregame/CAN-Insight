#include <SD.h>
#include <FlexCAN_T4.h>
#include "datatypes.h"
#include "config.h"


void print_log_filename();
void bus_config_to_str(CANBus_Config* config, char*sTmp);

class SD_CAN_Logger {
    public:
        uint32_t max_log_size;
        int start_log();
        void set_next_log_filename();
        void get_log_filename(char* name);
        SD_CAN_Logger();
        SD_CAN_Logger(char* _unit_type, char* _unit_number, CANBus_Config _can_configs[3], uint32_t _max_log_size);
        void can_frame_to_str(const CAN_message_t &msg, char* sTmp);
        void write_sd_line(char* line);
        static void flush_sd_file();

    private:
        void set_time_since_log_start_in_buffer(char* sTmp);
        static File data_file;
        char log_file_name[LOG_FILE_NAME_LENGTH] = DEFAULT_LOG_FILE_NAME;
        char single_can_log_config_str[160];
        char *__unit_type;
        char *__unit_number;
        unsigned long log_start_millis;

        CANBus_Config *__can_configs;

};