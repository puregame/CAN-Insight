#include <FlexCAN_T4.h>
#include <SdFat.h>
#include "datatypes.h"
#include "config.h"
#include "rgb_led.h"
#include "time_manager.h"
#include "can_log.h"
#include "config_manager.h"
#include "helpers.h"
#include <EEPROM.h>

extern SdFs sd;
extern SD_CAN_Logger sd_logger;

FsFile SD_CAN_Logger::data_file;

SD_CAN_Logger::SD_CAN_Logger(Config_Manager* _config){
  config = _config;
  // get the latest first and next log file numbers
  EEPROM.get(EEPROM_LOCATION_LOGGER_FIRST_LOG_NUM, first_log_file_number);
  EEPROM.get(EEPROM_LOCATION_LOGGER_NEXT_LOG_NUM, next_log_file_number);
  if (next_log_file_number > MAX_LOG_NUMBER | first_log_file_number > MAX_LOG_NUMBER){
    reset_log_file_numbers();
  }
  #ifdef DEBUG
    Serial.print("EEPROM first log file number: ");
    Serial.println(first_log_file_number);
    Serial.print("EEPROM next log file number: ");
    Serial.println(next_log_file_number);
  #endif
}

void SD_CAN_Logger::reset_log_file_numbers(){
  next_log_file_number = 1;
  first_log_file_number = 0;
  EEPROM.put(EEPROM_LOCATION_LOGGER_NEXT_LOG_NUM, next_log_file_number);
  EEPROM.put(EEPROM_LOCATION_LOGGER_FIRST_LOG_NUM, first_log_file_number);
  EEPROM.put(EEPROM_LOCATION_UPLOADER_NEXT_TO_TRY, uint16_t(1));
  EEPROM.put(EEPROM_LOCATION_UPLOADER_MAX_LOG, uint16_t(0));
}

void SD_CAN_Logger::flush_sd_file(){
  // check if write buffer is not empty, if it has content then write to the file then flush
  if (strlen(sd_logger.write_buffer) > 0){
    data_file.print(sd_logger.write_buffer);
    sd_logger.write_buffer[0]='\0'; // clear the write buffer
  }

  data_file.flush();
}

void SD_CAN_Logger::reopen_file(){
  #ifdef DEBUG
    Serial.println("Reopening CAN log data file.");
  #endif
  uint64_t pos = data_file.position();
  data_file.close();
  data_file.open(log_file_name, FILE_WRITE);
  data_file.seek(pos);
}

void SD_CAN_Logger::set_time_since_log_start_in_buffer(char* sTmp){
  sprintf(sTmp, "%3.3f", (millis()-log_start_millis)/1000.0);
}

int SD_CAN_Logger::start_log(){
  log_enabled = false;
  if (!check_sd_free_space()){ // before starting to log, check and free up SD card space
    // nothing else to do, logging is disabed, return
    return 0;
  }
  if (!set_next_log_filename()){
    // could not get a free log file name, return and do not log
    return 0;
  }
  #ifdef DEBUG
    Serial.print("Logging on file: ");
    Serial.println(log_file_name);
  #endif
  data_file.close();
  data_file = sd.open(log_file_name, FILE_WRITE);
  // set max log to upload to this log-1
  EEPROM.put(EEPROM_LOCATION_UPLOADER_MAX_LOG, next_log_file_number - 1);
  // if the file is available, write to it:
  if (data_file) {
    // print header file in the log
    data_file.print("{\"unit_type\": \"");
    data_file.print(config->unit_type);
    data_file.print("\", \"unit_number\": \"");
    data_file.print(config->unit_number);
    data_file.print("\", \"can_1\": ");
    single_can_log_config_str[0] = '\0';
    config->bus_config_to_str(0, single_can_log_config_str);
    data_file.print(single_can_log_config_str);
    single_can_log_config_str[0] = '\0';
    data_file.print(", \"can_2\": ");
    config->bus_config_to_str(1, single_can_log_config_str);
    data_file.print(single_can_log_config_str);
    single_can_log_config_str[0] = '\0';
    data_file.print(", \"can_3\": ");
    config->bus_config_to_str(2, single_can_log_config_str);
    data_file.print(single_can_log_config_str);
    single_can_log_config_str[0] = '\0';
    data_file.print(", \"log_start_time\": \"");
    char s_tmp[30];
    set_current_time_in_buffer(s_tmp);
    data_file.print(s_tmp);
    data_file.println("\"}");
    log_start_millis = millis();
    data_file.println(HEADER_CSV);
  }
  else{
    Serial.println("file not opened!");
    return 0;
  }
  data_file.flush();
  log_enabled=true;
  if (first_log_file_number == 0){
    // if this is the first time we started a log then set first log accordingly
    first_log_file_number = 1;
    EEPROM.put(EEPROM_LOCATION_LOGGER_FIRST_LOG_NUM, first_log_file_number);
  }
  return 1;
}

void SD_CAN_Logger::can_frame_to_str_log(const CAN_message_t &msg, char* sTmp){
  set_time_since_log_start_in_buffer(sTmp);
  sprintf(sTmp+strlen(sTmp), "-%d-", (unsigned int)msg.bus);


  sprintf(sTmp+strlen(sTmp), "%X#", (unsigned int)msg.id);
  byte len = min(msg.len, 8);
  for (int i=0; i<len; i++){
    sprintf(sTmp+strlen(sTmp), "%0.2X", msg.buf[i]);
  }
  strcat(sTmp, "\r\n");
}

void SD_CAN_Logger::can_frame_to_str_csv(const CAN_message_t &msg, char* sTmp){
  set_time_since_log_start_in_buffer(sTmp);
  sprintf(sTmp+strlen(sTmp), ",%d", (unsigned int)msg.bus);
  sprintf(sTmp+strlen(sTmp), ",%d", (unsigned int)msg.flags.extended);
  sprintf(sTmp+strlen(sTmp), ",%X", (unsigned int)msg.id);
  sprintf(sTmp+strlen(sTmp), ",%d", (unsigned int)msg.len);
  byte len = min(msg.len, 8);
  for (int i=0; i<len; i++){
    sprintf(sTmp+strlen(sTmp), ",%0.2X", msg.buf[i]);
  }
  strcat(sTmp, "\r\n");
}

int SD_CAN_Logger::get_current_log_count(){
  // return count of log files including log that we are currently logging to
  if (first_log_file_number > next_log_file_number)  
    // if next log file number is less than first log file number then logs have wrapped
    return MAX_LOG_NUMBER;
  return first_log_file_number - next_log_file_number + 1;
}

bool SD_CAN_Logger::set_next_log_filename(){
  // search for and set the log_file_name and next_log_file_number into self object
  // returns: none
  Serial.println("Getting next log file name.");

  // check if we should be overwriting old files
  if (overwriting_old_files){
    #ifdef DEBUG
      Serial.println("\tOverwriting old logs, shortcut");
    #endif
    // delete first log file and increment
    if (next_log_file_number == first_log_file_number){
      sprintf_num_to_logfile_name(next_log_file_number, log_file_name);
      sd.remove(log_file_name);
      next_log_file_number++;
      first_log_file_number++;
      EEPROM.put(EEPROM_LOCATION_LOGGER_NEXT_LOG_NUM, next_log_file_number);
      EEPROM.put(EEPROM_LOCATION_LOGGER_FIRST_LOG_NUM, first_log_file_number);
      return; // file has been removed, it can now be used
    }
  }

  // if first log file does not exist then assume SD card has been cleared and start logging at log 1
  sprintf_num_to_logfile_name(first_log_file_number, log_file_name);
  if (!sd.exists(log_file_name)){
    reset_log_file_numbers();
    sprintf_num_to_logfile_name(next_log_file_number, log_file_name);
    #ifdef DEBUG
      Serial.print("\tFirst log file does not exist, resetting, using file: ");
      Serial.println(log_file_name);
    #endif
    return true;
  }
  
  if (next_log_file_number > MAX_LOG_NUMBER){
    //wrap number around when it gets to MAX_LOG_NUMBER
    next_log_file_number = 1;
  }
  // first test existing file
  sprintf_num_to_logfile_name(next_log_file_number, log_file_name);

  if (!sd.exists(log_file_name)){ 
    // next log file that we tried does not exist then use this number, increment the next one and use this file name
    next_log_file_number++;
    EEPROM.put(EEPROM_LOCATION_LOGGER_NEXT_LOG_NUM, next_log_file_number);
    #ifdef DEBUG
      Serial.print("\tNext log file does not exist, using file: ");
      Serial.println(log_file_name);
    #endif
    return true; // return with this log_file_name as the one to use
  }
  else{
    // next log file that we tried does exist, what to do?
    // if next log file is 0 then look though all logs?
    uint16_t log_file_to_try = next_log_file_number;
    log_file_to_try++;
    #ifdef DEBUG
      Serial.println("\tNext log file does exist, looking for useable files");
    #endif
    do {
      sprintf_num_to_logfile_name(log_file_to_try, log_file_name);
      if (!sd.exists(log_file_name)){
        // this file does not exist, use it
        next_log_file_number = log_file_to_try+1;
        EEPROM.put(EEPROM_LOCATION_LOGGER_NEXT_LOG_NUM, next_log_file_number);
        return true;
      }
      log_file_to_try++;
      if (log_file_to_try > MAX_LOG_NUMBER){
        log_file_to_try = 1;
      }
      #ifdef DEBUG
        Serial.print("\tStill looking for useable files. to try: ");
        Serial.print(log_file_to_try);
        Serial.print(" first_log_number: ");
        Serial.println(first_log_file_number);
      #endif
    }
    while(log_file_to_try != first_log_file_number); // try all file numbers until we get back to first log file

    // if we get here then there are MAX_LOG_NUMBER log files, none available.
    Serial.println("All log files up to first log file are used, checking if we should overwrite from config");
    if (config->overwrite_logs){
      // delete the oldest log, use that log.
      // Short circuit this algorithm to overwrite next time a new log is needed
      // assume next_log_file_number is the one to delete
      sprintf_num_to_logfile_name(first_log_file_number, log_file_name);
      sd.remove(log_file_name);
      first_log_file_number ++;
      next_log_file_number = first_log_file_number;
      EEPROM.put(EEPROM_LOCATION_LOGGER_NEXT_LOG_NUM, next_log_file_number);
      EEPROM.put(EEPROM_LOCATION_LOGGER_FIRST_LOG_NUM, first_log_file_number);
      overwriting_old_files = true;
      return true; // file has been removed, it can now be used
    }
    else{
      // Stop logging
      Serial.println("Stopping all logging, will not overwrite logs");
      return false;
    }
  }
  Serial.println("unknown state, no logging");
  return false;
}

bool SD_CAN_Logger::check_sd_free_space(){
  // checks free space available on SD card, 
    // if free space is not at least 2*max_log_size and log overwrite is enabled 
      // then delete oldest files until 2* max_log_size is available on card
  // Returns: True if there is enough space, False otherwise
  uint32_t freeKB = sd.vol()->freeClusterCount();
  freeKB *= sd.vol()->blocksPerCluster()/2;
  uint32_t min_free_space = config->max_log_size/1000 * 1.5;
  char log_file_to_delete[LOG_FILE_NAME_LENGTH] = DEFAULT_LOG_FILE_NAME;
  sprintf_num_to_logfile_name(first_log_file_number, log_file_to_delete);
  #ifdef DEBUG
    Serial.print("Checking free space on SD Card: ");
    Serial.print(freeKB);
    Serial.println(" KB");
    Serial.print("Minimum Free Space: ");
    Serial.print(min_free_space);
    Serial.println(" KB");
  #endif
  while (freeKB < min_free_space){
    if (!config->overwrite_logs){
      // if we are not allowed to overwrite logs return 0
      #ifdef DEBUG
        Serial.println("SD Card almost full but not allowed to delete files. Will not log to SD.");
      #endif
      return 0;
    }
    // delete oldest log file
    #ifdef DEBUG
      Serial.print("Deleting log file: ");
      Serial.println(log_file_to_delete);
    #endif
    sd.remove(log_file_to_delete);
    // update first log file number
    first_log_file_number++;
    if (first_log_file_number > MAX_LOG_NUMBER){
      first_log_file_number = 1;
    }
    EEPROM.put(EEPROM_LOCATION_LOGGER_FIRST_LOG_NUM, first_log_file_number);
    sprintf_num_to_logfile_name(first_log_file_number, log_file_to_delete);
    // recalculate free space
    freeKB = sd.vol()->freeClusterCount();
    freeKB *= sd.vol()->blocksPerCluster()/2;
    #ifdef DEBUG
      Serial.print("New free space on SD Card: ");
      Serial.print(freeKB);
      Serial.println(" KB");
    #endif
  }
  return 1;
}

void SD_CAN_Logger::get_log_filename(char* name){
    strcpy(name, log_file_name);
}

void SD_CAN_Logger::print_end_log_line(){
  data_file.println(EOF_CAN_LOGFILE);
}

void SD_CAN_Logger::restart_logging(){
    #ifdef DEBUG
      Serial.println("\tRestarting logging");
    #endif
  start_log();
}

void SD_CAN_Logger::write_sd_line(char* line){
  // open the file.
  // if the file is available, write to it:
  if (!log_enabled)
    return;
  if (no_write_file){
    if (strlen(write_buffer) + strlen(line) > SD_WRITE_BUFFER_LEN){
      Serial.println("ERROR: sd file buffer overrun!");
      return;
    }
    if (strlen(write_buffer)+strlen(line) < SD_WRITE_BUFFER_LEN){
      // if there is enough space in the buffer then append the line, otherwise ignore
      strcat(write_buffer, line);
    }
  }
  else{
    if (data_file) {
      if (strlen(write_buffer) > 0){
        data_file.print(write_buffer);
        write_buffer[0]='\0'; // clear the write buffer
      }
      data_file.print(line);

      if (data_file.size() > max_log_size){
        // if the file is too big, write EOF_CAN_LOGFILE line and restart logging
        print_end_log_line();
        restart_logging();
      }
    }
    else{
      #ifdef DEBUG
        // if the file isn't open, pop up an error:
        Serial.println("file not opened! opening and trying again");
      #endif
      data_file = sd.open(log_file_name, FILE_WRITE);
      write_sd_line(line);
    }
  }
}
