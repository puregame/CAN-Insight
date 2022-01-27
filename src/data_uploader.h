#include "datatypes.h"
#include "config.h"
#include <ArduinoJson.h>
#include <Client.h>
#include <ArduinoHttpClient.h>


#ifndef data_uploader_

#define data_uploader_
class DataUploader{
  public:
    DataUploader(Client& in_internet_client,  char* server, int port);
    void upload_data();
    bool upload_file(char* file_name);
    void test_get_route(char* route_url);
    uint16_t next_log_to_upload;
  private:
    void get_logs_uploaded();
    bool increment_next_log_to_upload();
    void reset_next_log_to_upload();
    HttpClient http_client;
    Client* internet_client;
    char server[MAX_SERVER_URL_LEN];
    int port;
    uint16_t max_log_to_upload;
};
#endif
