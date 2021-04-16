
#include "config.h"
#include "datatypes.h"


#ifndef log_file_
#define log_file_
class LogFileMeta{
  public:
    LogFileMeta(char* file_name);
    char unit_number[UNIT_INFO_MAX_LEN];
    char unit_type[UNIT_INFO_MAX_LEN];
    char log_start_time[TIME_STRING_MAX_LEN];
  private:
};
#endif
