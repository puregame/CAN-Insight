#include "datatypes.h"
#include "config.h"
#include <ArduinoJson.h>


class Config_Manager{
    public:
        void set_default_can_config(uint8_t config_num);
        int read_config_file();
        void set_can_config_from_jsonobject(JsonObject json_obj, uint8_t config_num);
        void serial_print_bus_config_str(uint8_t config_num);
        CANBus_Config can_configs[3];
        char unit_type[UNIT_INFO_MAX_LEN];
        char unit_number[UNIT_INFO_MAX_LEN];
        uint32_t max_log_size = DEFAULT_MAX_LOG_FILE_SIZE;
    private:
        void bus_config_to_str(uint8_t config_num, char*sTmp);
};