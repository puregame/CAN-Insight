// datatypes
#include <stdint.h>

struct CANBus_Config {
  unsigned int baudrate;
  uint8_t port;
  bool log_std;
  bool log_ext;
  int id_filter_mask;
  int id_filter_value;
  char bus_name[20];
};