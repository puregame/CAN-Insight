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
extern Config_Manager config;

DataUploader::DataUploader(Client& in_internet_client, char* in_server, int in_port):
  http_client(in_internet_client, in_server, in_port){ //, internet_client(&in_internet_client){
  internet_client = &in_internet_client;
  strcpy(server, in_server);
  port = in_port;
  EEPROM.get(EEPROM_LOCATION_UPLOADER_MAX_LOG, max_log_to_upload);
  EEPROM.get(EEPROM_LOCATION_UPLOADER_NEXT_TO_TRY, next_log_to_upload);
  if (next_log_to_upload > max_log_to_upload){
    next_log_to_upload = 1;
    EEPROM.put(EEPROM_LOCATION_UPLOADER_NEXT_TO_TRY, next_log_to_upload);
  }
  #ifdef DEBUG
    Serial.print("EEPROM next log to upload: ");
    Serial.println(next_log_to_upload);
    Serial.print("EEPROM max log to upload: ");
    Serial.println(max_log_to_upload-1);
  #endif
  get_logs_uploaded();
};

void DataUploader::get_logs_uploaded(){
  // read EEPROM for latest file
  char file_name[LOG_FILE_NAME_LENGTH] = DEFAULT_LOG_FILE_NAME;
  sprintf_num_to_logfile_name(next_log_to_upload, file_name);

  // IF the file does not exist, or we are are starting at log zero then reset logs to upload to zero
  if (!sd.exists(file_name))
    reset_next_log_to_upload();
}

void DataUploader::reset_next_log_to_upload(){
  next_log_to_upload = 1;
  EEPROM.put(EEPROM_LOCATION_UPLOADER_NEXT_TO_TRY, next_log_to_upload);
}

bool DataUploader::increment_next_log_to_upload(){
  // increment first since this file has already been uploaded
  next_log_to_upload++;
  EEPROM.put(EEPROM_LOCATION_UPLOADER_NEXT_TO_TRY, next_log_to_upload);
  if (next_log_to_upload + 1 > max_log_to_upload){
    // if next log would be higher than logs currently ready for upload, do not increment
    return false;
  }
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

  // if the next file to be sent sent does not exist, start from zero
  if (!sd.exists(file_name)){
    #ifdef DEBUG
      Serial.print("File to be uploaded does not exist, file that does not exist: ");
      Serial.println(file_name);
    #endif
    return;
  }
  
  while (sd.exists(file_name)){
    // loop over file_name until file_name does not exist any more
    if (upload_file(file_name)){
      // returns true if file uploaded properly
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
  request_uri = request_uri + log_meta.unit_type + "&unit_number=" + log_meta.unit_number + "&log_time=" + log_meta.log_start_time + "&log_name=" + file_name;
  http_client.beginRequest();
  http_client.get(request_uri);
  http_client.endRequest();
  int status_code = http_client.responseStatusCode();
  #ifdef DEBUG
    Serial.print("Sent request for unit info, got status code: ");
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
          Serial.println("\tConnected to server");
          
          Serial.println("\tReading data file");
          sd_logger.no_write_file = true;
          FsFile send_file = sd.open(file_name, O_READ);
          uint64_t file_size = send_file.size();
          char file_size_str[40];
          sprintf(file_size_str, "%d", file_size);

          char request_header[1400] = "POST /data_file/?log_name=";
          strcat(request_header, file_name);
          strcat(request_header, "&unit_number=");
          strcat(request_header, log_meta.unit_number);
          strcat(request_header, "&log_time=");
          strcat(request_header, log_meta.log_start_time);
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
          Serial.println("\tSending data: ");
          can_log_timer.stop();
          uint32_t last_sd_write = millis();
          while (i < file_size){
            if (!internet_client->connected()){
              Serial.println("\rClient not connected!");
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
          Serial.println("\t\tSent entire file");
        #endif
        sd_logger.reopen_file();
        can_log_timer.start();
        sd_logger.no_write_file = false;
        // get HTTP status code
        if (internet_client->available()){
          char status[13];
          internet_client->readBytes(status, 12);
          status[12] = '\0';
          
          if (strcmp(&status[9], "200") == 0){
            #ifdef DEBUG
              Serial.println("\tStatus is 200, can increment log file to send and delete this file");
            #endif
            if (config.delete_uploaded_logs){
              sd.remove(file_name);
              #ifdef DEBUG
                Serial.print("\tDeleted file: ");
                Serial.println(file_name);
              #endif
              sd_logger.first_log_file_number++;
              EEPROM.put(EEPROM_LOCATION_LOGGER_FIRST_LOG_NUM, sd_logger.first_log_file_number)
              ;// delete log that just got uploaded
            }
          }
          else{
            // server did not return success, do not increment file to send!
            return false;
          }
        }
        internet_client->stop();
        return true;
      }
    }
    else if (status_code == 200){
      #ifdef DEBUG
        Serial.println("\t File already exists, will not upload");
      #endif
      return true;
    }
    else if (status_code == HTTP_ERROR_TIMED_OUT){
      #ifdef DEBUG
        Serial.println("\tCould not connect to server, timeout");
      #endif
      return false;
    }
    else
      Serial.println("\tWill not upload file, other error");
      return false;
}