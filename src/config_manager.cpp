#include "Arduino.h"
#include "config.h"
#include "config_manager.h"
#include <ArduinoJson.h>
#include <SD.h>

void Config_Manager::bus_config_to_str(uint8_t config_num, char*sTmp){
  strcat(sTmp, "{\"bus_number\": ");
  sprintf(sTmp+strlen(sTmp), "%d", (unsigned int)can_configs[config_num].port);
  strcat(sTmp, ", \"bus_name\": \"");
  strcat(sTmp, can_configs[config_num].bus_name);
  strcat(sTmp, "\", \"baudrate\": ");
  sprintf(sTmp+strlen(sTmp), "%d", can_configs[config_num].baudrate);
  strcat(sTmp, ", \"log_std\": ");
  sprintf(sTmp+strlen(sTmp), "%d", can_configs[config_num].log_std);
  strcat(sTmp, ", \"log_ext\": ");
  sprintf(sTmp+strlen(sTmp), "%d", can_configs[config_num].log_ext);
  strcat(sTmp, ", \"id_filter_mask\": ");
  sprintf(sTmp+strlen(sTmp), "%d", can_configs[config_num].id_filter_mask);
  strcat(sTmp, ", \"id_filter_value\": ");
  sprintf(sTmp+strlen(sTmp), "%d", can_configs[config_num].id_filter_value);
  strcat(sTmp, ", \"log_enabled\": ");
  sprintf(sTmp+strlen(sTmp), "%d", can_configs[config_num].log_enabled);
  strcat(sTmp, "}");
}

void Config_Manager::set_default_can_config(uint8_t config_num){
  Serial.print("Setting default config for bus: ");
  Serial.println(can_configs[config_num].port);
  sprintf(can_configs[config_num].bus_name, "CAN%d", can_configs[config_num].port);
  can_configs[config_num].baudrate=DEFAULT_BAUD_RATE;
  can_configs[config_num].id_filter_mask=0;
  can_configs[config_num].id_filter_value=0;
  can_configs[config_num].log_ext=true;
  can_configs[config_num].log_std=true;
}

int Config_Manager::read_config_file() {
  Serial.println("reading Config file");
  File config_file = SD.open(CONFIG_FILE_NAME, FILE_READ);
  StaticJsonDocument<512> config_doc;
  DeserializationError error = deserializeJson(config_doc, config_file);
  config_file.close();
  JsonObject config_root = config_doc.as<JsonObject>();
  if (error){
    Serial.println(F("Failed to read file, using default configuration"));
    set_default_can_config(0);
    set_default_can_config(1);
    set_default_can_config(2);
    return 1;
  }
  JsonObject temp_object;
  if (config_root.containsKey("max_file_size")){
    max_log_size = config_root["max_file_size"];
  }
  if (config_root.containsKey("unit_type")){
    strlcpy(unit_type, config_root["unit_type"] | "", UNIT_INFO_MAX_LEN);
  }
  if (config_root.containsKey("unit_number")){
    strlcpy(unit_number, config_root["unit_number"] | "", UNIT_INFO_MAX_LEN);
  }

  if (config_root.containsKey("can1")){ // if can1 key exists then process it, otherwise set it to default confi
    temp_object = config_root["can1"];
    set_can_config_from_jsonobject(temp_object, 0);
  }
  else 
    set_default_can_config(0);

  if (config_root.containsKey("can2")){
    temp_object = config_root["can2"];
    set_can_config_from_jsonobject(temp_object, 1);
  }
  else
    set_default_can_config(1);

  if (config_root.containsKey("can3")){
    temp_object = config_root["can3"];
    set_can_config_from_jsonobject(temp_object, 2);
  }
  else
    set_default_can_config(2);
  return 1;
}

void Config_Manager::set_can_config_from_jsonobject(JsonObject json_obj, uint8_t config_num){
  can_configs[config_num].baudrate = json_obj["baudrate"] | DEFAULT_BAUD_RATE;
  char bus_str[5];
  sprintf(bus_str, "CAN%d", can_configs[config_num].port);
  strlcpy(can_configs[config_num].bus_name, json_obj["bus_name"] | bus_str, sizeof(can_configs[config_num].bus_name));
  can_configs[config_num].log_ext = json_obj["log_extended_frames"] | true;
  can_configs[config_num].log_std = json_obj["log_standard_frames"] | true;
  can_configs[config_num].id_filter_mask = json_obj["id_filter_mask"] | 0;
  can_configs[config_num].id_filter_value = json_obj["id_filter_value"] | 0;
}

void Config_Manager::serial_print_bus_config_str(uint8_t config_num){
  char s_tmp[200] = "";
  bus_config_to_str(config_num, s_tmp);
  Serial.println(s_tmp);
}
