
// #include <ArduinoHttpClient.h>
#include "config.h"
#include "data_uploader.h"
#include "helpers.h"
#include <SD.h>
#include <EEPROM.h>

DataUploader::DataUploader(Client& in_internet_client, char* in_server, int in_port):
    http_client(in_internet_client, in_server, in_port){ //, internet_client(&in_internet_client){
    internet_client = &in_internet_client;
    strcpy(server, in_server);
    port = in_port;
    get_logs_uploaded();
};

void DataUploader::get_logs_uploaded(){
  // read EEPROM for latest file
  int file_to_try;
  EEPROM.get(EEPROM_LOGs_UPLOADED_LOCATION, file_to_try);
  char file_name[LOG_FILE_NAME_LENGTH] = "CAN_000.log";
  sprintf_num_to_logfile_name(file_to_try, file_name);

  // check if that CAN file actually exists
  if (SD.exists(file_name))
    next_log_to_upload = file_to_try+1;
  else
    next_log_to_upload = 0;
  // if it does exist than it is the latest one that got uploaded
  // if not then reset logs_uploaded to 0

}

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
            send_file.read(buf, FILE_BUF_LEN);
            internet_client->write(buf, FILE_BUF_LEN);
            i += FILE_BUF_LEN;
        }
      }
    }
    else
      Serial.println("Will not upload file");
}