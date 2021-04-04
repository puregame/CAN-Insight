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
  private:
    HttpClient http_client;
    Client* internet_client;
    char server[MAX_SERVER_LEN];
    int port;
};
#endif
