// datatypes

struct CANBus_Config {
  uint32_t baudrate;
  char port;
  bool log_std;
  bool log_ext;
  int id_filter_mask;
  int id_filter_value;
  char bus_name[20];
};