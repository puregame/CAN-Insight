#include <SdFat.h>
#include <FlexCAN_T4.h>
#include "datatypes.h"
#include "config.h"
#include "config_manager.h"

void print_log_filename();

class SD_CAN_Logger {
    public:
        SD_CAN_Logger(Config_Manager* _config);
        uint32_t max_log_size;
        int start_log();
        void get_log_filename(char* name);
        void can_frame_to_str_csv(const CAN_message_t &msg, char* sTmp);
        void can_frame_to_str_log(const CAN_message_t &msg, char* sTmp);
        void write_sd_line(char* line);
        void reopen_file();
        static void flush_sd_file();
        void restart_logging();
        uint16_t next_log_file_number = 1;
        uint16_t first_log_file_number = 0;
        bool no_write_file = false;
        bool check_sd_free_space();
        int get_current_log_count();
    private:
        void reset_log_file_numbers();
        bool set_next_log_filename();
        char write_buffer[SD_WRITE_BUFFER_LEN];
        void print_end_log_line();
        void set_time_since_log_start_in_buffer(char* sTmp);
        static FsFile data_file;
        char log_file_name[LOG_FILE_NAME_LENGTH] = DEFAULT_LOG_FILE_NAME;
        char single_can_log_config_str[160];
        unsigned long log_start_millis;
        Config_Manager* config;
        bool log_enabled = true;
        bool overwriting_old_files = false;
};
