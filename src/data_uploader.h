#include "datatypes.h"
#include "config.h"
#include <ArduinoJson.h>
#include <Client.h>
#include <ArduinoHttpClient.h>


#ifndef data_uploader_

#define data_uploader_
class DataUploader{
  public:
    DataUploader(Client& in_internet_client,  char* server, int port, int _max_log_to_upload);
    void upload_data();
    bool upload_file(char* file_name);
    void test_get_route(char* route_url);
    int next_log_to_upload;
  private:
    void get_logs_uploaded();
    bool increment_next_log_to_upload();
    void set_next_log_to_upload_to_zero();
    HttpClient http_client;
    Client* internet_client;
    char server[MAX_SERVER_LEN];
    int port;
    int max_log_to_upload;
};
#endif
