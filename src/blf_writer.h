#include <SdFat.h>
#include <FlexCAN_T4.h>
#include "datatypes.h"
#include "config.h"
#include "config_manager.h"

void print_log_filename();

#define FILE_HEADER_SIZE 144

FsFile SD_CAN_Logger::data_file;

class BLF_Writer {
    public:
        BLF_Writer(int channel);

    private:
        void write_header(int filesize);
        static File data_file;
        int channel = 0;
        int object_count = 0;
        int size = FILE_HEADER_SIZE;
};