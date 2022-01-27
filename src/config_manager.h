#include "datatypes.h"
#include "config.h"
#include <ArduinoJson.h>

#ifndef config_manager_
#define config_manager_

class Config_Manager{
    public:
        void set_default_can_config(uint8_t config_num);
        int read_config_file();
        void set_can_config_from_jsonobject(JsonObject json_obj, uint8_t config_num);
        void serial_print_bus_config_str(uint8_t config_num);
        void set_new_wifi_net(char* ssid, char* pass);
        CANBus_Config can_configs[3];
        Wifi_Network wifi_nets[MAX_SAVED_NETWORK_COUNT];
        int num_wifi_nets;
        char unit_type[UNIT_INFO_MAX_LEN];
        char unit_number[UNIT_INFO_MAX_LEN];
        uint32_t max_log_size = DEFAULT_MAX_LOG_FILE_SIZE;
        void bus_config_to_str(uint8_t config_num, char*sTmp);
        bool wifi_enabled;
        char server[SERVER_MAX_LEN];
        uint16_t port;
        bool overwrite_logs;
        bool delete_uploaded_logs;
    private:
};

#endif
