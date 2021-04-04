
// #include <ArduinoHttpClient.h>
#include "config.h"
#include "data_uploader.h"
#include <SD.h>

DataUploader::DataUploader(Client& in_internet_client, char* in_server, int in_port):
    http_client(in_internet_client, in_server, in_port){ //, internet_client(&in_internet_client){
    internet_client = &in_internet_client;
    strcpy(server, in_server);
    port = in_port;
};

void DataUploader::upload_data(){

    String request_uri = "/data_file/?unit_type=";
    request_uri = request_uri+"test"+"&unit_number=" + "test" + "&log_time=" + "2021-03-06T12-46-01-466Z";
    http_client.beginRequest();
    http_client.get(request_uri);
    http_client.endRequest();
    int status_code = http_client.responseStatusCode();

    if (status_code == 404) {
      Serial.println("Will upload file");
      delay(100);
        if (internet_client->connect(server, port)) {
          Serial.println("- connected to server");
      #define LOG_NAME "CAN_001.LOG"
          internet_client->print("POST /data_file/?log_name=");
          internet_client->print(LOG_NAME);
          internet_client->println(" HTTP/1.1");
          internet_client->println("User-Agent: Arduino/1.0");
      //    internet_client->println("Content-Type: application/x-www-form-urlencoded");
          internet_client->println("Content-Type: text/plain");
          internet_client->print("Content-Length: ");
        
          Serial.println("reading data file");
          File send_file = SD.open(LOG_NAME, FILE_READ);
          internet_client->println(send_file.size());
          internet_client->println();
          // for (int i = 0; i < send_file.size(); i++){
          //   internet_client->write(send_file.read());
          // }
          #define FILE_BUF_LEN 1300
          char buf[FILE_BUF_LEN] = "";
          unsigned int i = 0;
          while (i < send_file.size()){
            buf[0] = '\0'; // clear the buffer before reading more data
            send_file.read(buf, FILE_BUF_LEN-2);
            internet_client->write(buf, FILE_BUF_LEN-2);
            i += FILE_BUF_LEN-2;
        }
      }
    }
    else
      Serial.println("Will not upload file");
}