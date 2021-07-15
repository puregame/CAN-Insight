
// #include <ArduinoHttpClient.h>
#include "config.h"
#include "data_uploader.h"
#include "helpers.h"
#include "log_file.h"
#include <SdFat.h>
#include <EEPROM.h>
#include "can_log.h"

extern SdFs sd;
#include "TeensyTimerTool.h"
extern TeensyTimerTool::PeriodicTimer can_log_timer;
extern SD_CAN_Logger sd_logger;

DataUploader::DataUploader(Client& in_internet_client, char* in_server, int in_port, int _max_log_to_upload):
    http_client(in_internet_client, in_server, in_port){ //, internet_client(&in_internet_client){
    internet_client = &in_internet_client;
    max_log_to_upload = _max_log_to_upload;
    strcpy(server, in_server);
    port = in_port;
    get_logs_uploaded();
};

void DataUploader::get_logs_uploaded(){
  // read EEPROM for latest file
  EEPROM.get(EEPROM_LOGs_UPLOADED_LOCATION, next_log_to_upload);
  char file_name[LOG_FILE_NAME_LENGTH] = DEFAULT_LOG_FILE_NAME;
  sprintf_num_to_logfile_name(next_log_to_upload, file_name);

  // IF the file does not exist, or we are are starting at log zero then reset logs to upload to zero
  if (!sd.exists(file_name) | (max_log_to_upload < 0))
    set_next_log_to_upload_to_zero();
}

void DataUploader::set_next_log_to_upload_to_zero(){
  next_log_to_upload = 0;
  EEPROM.put(EEPROM_LOGs_UPLOADED_LOCATION, next_log_to_upload);
}

bool DataUploader::increment_next_log_to_upload(){
  if (next_log_to_upload >= max_log_to_upload){
    // if next log would be higher than logs currently ready for upload, do not increment
    return false;
  }
  next_log_to_upload++;
  EEPROM.put(EEPROM_LOGs_UPLOADED_LOCATION, next_log_to_upload);
  # ifdef DEBUG
    Serial.print("Incrementing next log to upload to: ");
    Serial.println(next_log_to_upload);
  #endif
  return true;
}

void DataUploader::test_get_route(char* route_url){
  Serial.print("performing test get on url: ");
  Serial.println(route_url);
  http_client.beginRequest();
  http_client.get(route_url);
  http_client.endRequest();
  Serial.print("Got status code: ");
  Serial.println(http_client.responseStatusCode());
  Serial.print("HTTP response: ");
  Serial.println(http_client.responseBody());
}

void DataUploader::upload_data(){
  // *********************** WHY IS THIS UPLOADING THE SAME FILE THAT IT ALREADY DID LAST RUN *********************
  // assume in_internet_client is already connected to a network
  #ifdef DEBUG
    next_log_to_upload=0; // set next log to upload to zero, upload all logs every time for debug
    Serial.print("Starting upload_data with next file: ");
    Serial.println(next_log_to_upload);
    Serial.print("Will not upload logs above: ");
    Serial.println(max_log_to_upload);
  #endif
  if (next_log_to_upload > max_log_to_upload){
    // if next log would be higher than logs currently ready for upload, do not upload
    #ifdef DEBUG
      Serial.println("Next log would be too high, exiting");
    #endif
    return;
  }
  char file_name[LOG_FILE_NAME_LENGTH] = DEFAULT_LOG_FILE_NAME;
  sprintf_num_to_logfile_name(next_log_to_upload, file_name);

  // if the last file that has been sent does not exist, start from zero
  if (!sd.exists(file_name)){
    #ifdef DEBUG
      Serial.print("Latest file does not exist, setting next file to zero, file that does not exist: ");
      Serial.println(file_name);
    #endif
    set_next_log_to_upload_to_zero();
    return;
  }
  else {
    #ifdef DEBUG
      Serial.print("latest file to upload does exist: ");
      Serial.println(file_name);
    #endif
  }
  while (sd.exists(file_name)){
    // loop over file_name until file_name does not exist any more
    if (upload_file(file_name)){
      if (!increment_next_log_to_upload()){
        // increment failed because next log file would be too high
        #ifdef DEBUG
          Serial.println("Exiting loop, done all current uploads");
        #endif
        break;
      }
      sprintf_num_to_logfile_name(next_log_to_upload, file_name);
    }
    else{
      // if upload was not successful then do not try other uploads
      #ifdef DEBUG
        Serial.println("Exiting loop forcefully, upload did not work");
      #endif
      break;
    }
  }
  #ifdef DEBUG
    Serial.println("loop uploading data exited");
  #endif
}

bool DataUploader::upload_file(char* file_name){
  // open file, get metadata (unit type, unit number, log time)
  LogFileMeta log_meta = LogFileMeta(file_name);

  // do get request to see if file already exists
  #ifdef DEBUG
    Serial.print("Starting upload for file: ");
    Serial.println(file_name);
    Serial.println("Sending request for unit info");
    delay(100);
  #endif
  String request_uri = "/data_file/?unit_type=";
  request_uri = request_uri + log_meta.unit_type + "&unit_number=" + log_meta.unit_number + "&log_time=" + log_meta.log_start_time;
  http_client.beginRequest();
  http_client.get(request_uri);
  http_client.endRequest();
  int status_code = http_client.responseStatusCode();
  #ifdef DEBUG
    Serial.print("Sent request for unit info, got status code");
    Serial.println(status_code);
    delay(100);
  #endif

  // if request returns 404, then upload the file
    if (status_code == 404) {
      #ifdef DEBUG
        Serial.println("Got 404 from check if file exists request, will upload file");
        delay(100);
      #endif
        if (internet_client->connect(server, port)) {
          Serial.println("- connected to server");
          
          Serial.println("reading data file");
          sd_logger.no_write_file = true;
          FsFile send_file = sd.open(file_name, O_READ);
          uint64_t file_size = send_file.size();
          char file_size_str[40];
          sprintf(file_size_str, "%d", file_size);

          char request_header[1400] = "POST /data_file/?log_name=";
          strcat(request_header, file_name);
          strcat(request_header, " HTTP/1.1\nUser-Agent: Arduino/1.0\nContent-Type: text/plain\nContent-Length: ");
          strcat(request_header, file_size_str);
          strcat(request_header, "\n");
        
          internet_client->println(request_header);
          #define TCP_PER_FILE_READ 200
          #define TCP_LEN 1200
          #define FILE_BUF_LEN TCP_LEN * TCP_PER_FILE_READ
          uint8_t buf[FILE_BUF_LEN] = "";
          uint64_t i = 0;
          uint64_t file_pos = 0;
          Serial.print("Sending data:");
          can_log_timer.stop();
          uint32_t last_sd_write = millis();
          while (i < file_size){
            if (!internet_client->connected()){
              Serial.println("Client not connected!");
              can_log_timer.start();
              return false;
            }
            send_file = sd.open(file_name, O_READ);
            buf[0] = '\0'; // clear the buffer before reading more data
            send_file.seek(file_pos);
            send_file.read(buf, FILE_BUF_LEN);
            file_pos = send_file.position();
            send_file.close();
            if ((millis() - last_sd_write) > 500){
              // every second flush SD file
              sd_logger.reopen_file();
              sd_logger.flush_sd_file();
              last_sd_write = millis();
            }
            uint64_t j = 0;
            while (j < FILE_BUF_LEN){
              internet_client->write(buf + j, TCP_LEN);
              i += TCP_LEN;
              j += TCP_LEN;
            }
        }
        #ifdef DEBUG
          Serial.println("Sent entire file");
        #endif
        sd_logger.reopen_file();
        can_log_timer.start();
        sd_logger.no_write_file = true;
        return true;
      }
    }
    else if (status_code == 200){
      #ifdef DEBUG
        Serial.println("File already exists, will not upload");
      #endif
      return true;
    }
    else if (status_code == HTTP_ERROR_TIMED_OUT){
      #ifdef DEBUG
        Serial.println("Could not connect to server, timeout");
      #endif
      return false;
    }
    else
      Serial.println("Will not upload file, other error");
      return false;
}