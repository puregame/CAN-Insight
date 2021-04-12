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
  StaticJsonDocument<CONFIG_FILE_JSON_SIZE_BYTES> config_doc;
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
    #ifdef DEBUG
      Serial.print("max log size: ");
      Serial.print(max_log_size);
    #endif
  }
  if (config_root.containsKey("unit_type")){
    strlcpy(unit_type, config_root["unit_type"] | "", UNIT_INFO_MAX_LEN);
    #ifdef DEBUG
      Serial.print("Unit type: ");
      Serial.println(unit_type);
    #endif
  }
  if (config_root.containsKey("unit_number")){
    strlcpy(unit_number, config_root["unit_number"] | "", UNIT_INFO_MAX_LEN);
    #ifdef DEBUG
      Serial.print("Unit number: ");
      Serial.println(unit_number);
    #endif
  }

  if (config_root.containsKey("can1")){ // if can1 key exists then process it, otherwise set it to default confi
    temp_object = config_root["can1"];
    set_can_config_from_jsonobject(temp_object, 0);
  }
  else 
    set_default_can_config(0);

  if (config_root.containsKey("can2")){ // process CAN2
    temp_object = config_root["can2"];
    set_can_config_from_jsonobject(temp_object, 1);
  }
  else
    set_default_can_config(1);

  if (config_root.containsKey("can3")){ // process CAN3
    temp_object = config_root["can3"];
    set_can_config_from_jsonobject(temp_object, 2);
  }
  else
    set_default_can_config(2);
  
  if (config_root.containsKey("wifi_enable")){
    #ifdef DEBUG
      Serial.println("got wifi enable in config file");
      delay(100);
    #endif
    wifi_enabled = config_root["wifi_enable"] | false;
    strlcpy(server, config_root["server"] | "", SERVER_MAX_LEN );
    port = config_root["port"] | 80;
    #ifdef DEBUG
      Serial.print("Server: ");
      Serial.println(server);
      Serial.print("port: ");
      Serial.println(port);
    #endif
  }

  #ifdef DEBUG
    delay(100);
  #endif

  if (config_root.containsKey("wifi_networks")){ // process wifi
    #ifdef DEBUG
      Serial.println("Got wifi networks key in config file");
    #endif
    JsonArray temp_array = config_root["wifi_networks"].as<JsonArray>();
    // loop through the wifi configs and add them to the wifi config array
    for (JsonArray::iterator it=temp_array.begin(); it!=temp_array.end(); ++it) {
      temp_object = it->as<JsonObject>();
      #ifdef DEBUG
        Serial.print("Wifi in config -  ssid: ");
        Serial.print(temp_object["ssid"] | "");
        Serial.print(" password: ");
        Serial.println(temp_object["password"] | "");
        delay(100);
      #endif
      set_new_wifi_net(temp_object["ssid"] | "", temp_object["password"] | "");
    }
  }
  return 1;
}

void Config_Manager::set_new_wifi_net(char* ssid, char* password){
  if (num_wifi_nets >= MAX_SAVED_NETWORK_COUNT){
    #ifdef DEBUG
      Serial.print("Warning, too many wifi networks, not saving: ");
      Serial.println(ssid);
      delay(100);
    #endif
  }
  else {
    strcpy(wifi_nets[num_wifi_nets].ssid, ssid);
    strcpy(wifi_nets[num_wifi_nets].password, password);
    num_wifi_nets ++;
  }
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
