#include "Arduino.h"
#include <FlexCAN_T4.h>
#include <ArduinoJson.h>
#include <SD.h>
#include "config.h"
#include "datatypes.h"
#include "rgb_led.h"
#include "time_manager.h"
#include "can_log.h"

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1; //orig RX_SIZE_256 TX_SIZE_64
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can2; //orig RX_SIZE_256 TX_SIZE_64
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can3; //orig RX_SIZE_256 TX_SIZE_64

// see TimeTeensy3 example for how to use time library

// use https://github.com/tonton81/FlexCAN_T4 library for receiving CAN data

// look into: https://github.com/greiman/SdFat-beta/blob/master/examples/ExFatLogger/ExFatLogger.ino

// configuration variables
CANBus_Config can_config_1; // initilize three canbus configurations for teensy 4.1
CANBus_Config can_config_2;
CANBus_Config can_config_3;
char unit_type[UNIT_INFO_MAX_LEN];
char unit_number[UNIT_INFO_MAX_LEN];
uint32_t max_log_size = DEFAULT_MAX_LOG_FILE_SIZE;

SD_CAN_Logger sd_logger;
System_Status status;

// ******* Setup timers
#include "TeensyTimerTool.h"
using namespace TeensyTimerTool;
// This does not work when timelib.h is also included !
//// timer for LED blinking
Timer t1; // generate a timer from the pool (Pool: 2xGPT, 16xTMR(QUAD), 20xTCK)
Timer t2; // generate a timer from the pool (Pool: 2xGPT, 16xTMR(QUAD), 20xTCK)

// ***** setup sd card
// setup sd card datalogging
// #include "SdFat.h"
// #include <SPI.h>
// SdFat sd_fat;

const int chipSelect = BUILTIN_SDCARD;
uint16_t log_number = 0;
// ** begin functions

void blink_builtin_led()
{
    digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));    
}


void set_default_config(CANBus_Config* config){
  Serial.print("Setting default config for bus: ");
  Serial.println(config->port);
  sprintf(config->bus_name, "CAN%d", config->port);
  config->baudrate=DEFAULT_BAUD_RATE;
  config->id_filter_mask=0;
  config->id_filter_value=0;
  config->log_ext=true;
  config->log_std=true;
}

void set_can_config_from_jsonobject(JsonObject json_obj, CANBus_Config* config){
  config->baudrate = json_obj["baudrate"] | DEFAULT_BAUD_RATE;
  char bus_str[5];
  sprintf(bus_str, "CAN%d", config->port);
  strlcpy(config->bus_name, json_obj["bus_name"] | bus_str, sizeof(config->bus_name));
  config->log_ext = json_obj["log_extended_frames"] | true;
  config->log_std = json_obj["log_standard_frames"] | true;
  config->id_filter_mask = json_obj["id_filter_mask"] | 0;
  config->id_filter_value = json_obj["id_filter_value"] | 0;
  ;

}

int read_config_file() {
  Serial.println("reading Config file");
  File config_file = SD.open(CONFIG_FILE_NAME, FILE_READ);
  StaticJsonDocument<512> config_doc;
  DeserializationError error = deserializeJson(config_doc, config_file);
  config_file.close();
  JsonObject config_root = config_doc.as<JsonObject>();
  if (error){
    Serial.println(F("Failed to read file, using default configuration"));
    set_default_config(&can_config_1);
    set_default_config(&can_config_2);
    set_default_config(&can_config_3);
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
    set_can_config_from_jsonobject(temp_object, &can_config_1);
  }
  else 
    set_default_config(&can_config_1);
  if (config_root.containsKey("can2")){
    temp_object = config_root["can2"];
    set_can_config_from_jsonobject(temp_object, &can_config_2);
  }
  else
    set_default_config(&can_config_2);
  if (config_root.containsKey("can3")){
    temp_object = config_root["can3"];
    set_can_config_from_jsonobject(temp_object, &can_config_3);
  }
  else
    set_default_config(&can_config_3);
  return 1;
}

void can_callback(const CAN_message_t &msg) {
  // filter by extended and standard
  //filter by ID filter
    // id filtering in akpc CAN_Logger uses if ((rxmsg.EID & iFilterMask) != (iFilterValue & iFilterMask)) continue;
  char temp_str[128];
  sd_logger.can_frame_to_str(msg, temp_str);
  #ifdef DEBUG
    Serial.print("Got CAN message: ");
    Serial.print(temp_str);
  #endif
  sd_logger.write_sd_line(temp_str);
}

void setup_from_sd_card(){
  if (!SD.begin(chipSelect)) {
    status = no_sd;
    set_led_from_status(status);
    Serial.println("Card failed, or not present");
    return;
  }
  read_time_file();
  // if (!sd_fat.begin(chipSelect, SPI_HALF_SPEED)) sd.initErrorHalt();

  if (!read_config_file()) Serial.println("Config File read error!");
  char single_can_log_config_str[160];
  single_can_log_config_str[0] = '\0';
  bus_config_to_str(&can_config_1, single_can_log_config_str);
  Serial.println(single_can_log_config_str);
  single_can_log_config_str[0] = '\0';
  bus_config_to_str(&can_config_2, single_can_log_config_str);
  Serial.println(single_can_log_config_str);
  single_can_log_config_str[0] = '\0';
  bus_config_to_str(&can_config_3, single_can_log_config_str);
  Serial.println(single_can_log_config_str);
  single_can_log_config_str[0] = '\0';
  
  sd_logger = SD_CAN_Logger(unit_type, unit_number, &can_config_1, &can_config_2, &can_config_3, max_log_size);
  
  sd_logger.set_next_log_filename();
  Serial.print("Logging on file:");
  char log_name[20];
  sd_logger.get_log_filename(log_name);
  Serial.println(log_name);
  sd_logger.start_log();
  t2.beginPeriodic(sd_logger.flush_sd_file, 1'000'000); // flush sd file every second

  // setup CANBus
  if (can_config_1.log_enabled){
    Serial.println("Beginning CAN1");
    Can1.setBaudRate(can_config_1.baudrate*1000);
    Can1.enableFIFO();
    Can1.enableFIFOInterrupt();
    Can1.onReceive(can_callback);
    Can1.begin();
  }
  if (can_config_2.log_enabled){
    Serial.println("Beginning CAN2");
    Can2.setBaudRate(can_config_2.baudrate*1000);
    Can2.enableFIFO();
    Can2.enableFIFOInterrupt();
    Can2.onReceive(can_callback);
    Can2.begin();
  }
  if (can_config_3.log_enabled){
    Serial.println("Beginning CAN3");
    Can3.setBaudRate(can_config_3.baudrate*1000);
    Can3.enableFIFO();
    Can3.enableFIFOInterrupt();
    Can3.onReceive(can_callback);
    Can3.begin();
  }
  status = waiting_for_data;
  set_led_from_status(status);
}

void setup() {
  setup_led();
  can_config_1.port = 1;
  can_config_2.port = 2;
  can_config_3.port = 3;
  t1.beginPeriodic(blink_builtin_led, 100'000); // 100ms blink every 100ms
  Serial.begin(115200); 
  Serial.println("Starting Program");
  
  #ifdef DEBUG
    delay(2000); // delay 1 seconds to allow computer to open serial connection
  #endif
  SdFile::dateTimeCallback(dateTime);
  set_sync_provider_teensy3();

  if (! rtc_sync_complete()) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("Sync with RTC complete");
  }
  Serial.print("Current Time: ");
  serial_print_current_time();

  setup_from_sd_card();
}

unsigned long target_time = 0L ;
#define ONE_SECOND_PERIOD 1*1000
#define TEN_SECOND_PERIOD 10*1000

void loop ()
{
  Can3.events();
  if (millis () - target_time >= ONE_SECOND_PERIOD)
  {
    target_time += ONE_SECOND_PERIOD ;   // change scheduled time exactly, no slippage will happen
    if (status == no_sd){
      setup_from_sd_card();
    }
  }

  check_serial_time();
}
