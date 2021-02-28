
#include "Arduino.h"
void set_current_time_in_buffer(char* str_addr);
void read_time_file();
void dateTime(uint16_t* date, uint16_t* time);
void serial_print_current_time();
unsigned long processSyncMessage();
void check_serial_time();
void read_time_file();
bool rtc_sync_complete();
void set_sync_provider_teensy3();