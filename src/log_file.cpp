#include "config.h"
#include "datatypes.h"
#include "log_file.h"
#include <SD.h>
#include <ArduinoJson.h>

LogFileMeta::LogFileMeta(char* file_name){
  // open file, read first line
  char file_first_line[2000] = "";
  File log_file = SD.open(file_name, FILE_READ);
  char cr[2] = "A";
  for(unsigned int i = 0; i < (2000 -1); i++){
    cr[0] = log_file.read();
    if(cr[0] != '\n'){
      strcat(file_first_line, cr);
    }
    else
      break;
  }
  log_file.close();

  // parse as json
  StaticJsonDocument<CONFIG_FILE_JSON_SIZE_BYTES> config_doc;
  DeserializationError error = deserializeJson(config_doc, file_first_line);
  if (error){
    Serial.print("JSON file serialization error for log file: ");
    Serial.println(file_name);
    return;
  }
  JsonObject config_root = config_doc.as<JsonObject>();

  // set log file metadata from the json
  strlcpy(unit_number, config_root["unit_number"] | "", UNIT_INFO_MAX_LEN );
  strlcpy(unit_type, config_root["unit_type"] | "", UNIT_INFO_MAX_LEN );
  strlcpy(log_start_time, config_root["log_start_time"] | "", TIME_STRING_MAX_LEN );

  #ifdef DEBUG
    Serial.print("Read meta for file: ");
    Serial.println(file_name);
    Serial.print("\t unit_number: ");
    Serial.println(unit_number);
    Serial.print("\t unit_type: ");
    Serial.println(unit_type);
    Serial.print("\t log_start_time: ");
    Serial.println(log_start_time);
  #endif
}