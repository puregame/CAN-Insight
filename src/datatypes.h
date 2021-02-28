// datatypes
#include <stdint.h>

#ifndef canbus_config
#define canbus_config
struct CANBus_Config {
  unsigned int baudrate;
  uint8_t port;
  bool log_std;
  bool log_ext;
  int id_filter_mask;
  int id_filter_value;
  char bus_name[21];
  bool log_enabled = true;
};
enum System_Status{boot, no_sd, waiting_for_data, writing_sd};
#endif