#include "Arduino.h"
#include <FlexCAN_T4.h>
#include <SD.h>
#include "config.h"
#include "datatypes.h"
#include "rgb_led.h"
#include "time_manager.h"
#include "config_manager.h"
#include "can_log.h"

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1; //orig RX_SIZE_256 TX_SIZE_64
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can2; //orig RX_SIZE_256 TX_SIZE_64
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can3; //orig RX_SIZE_256 TX_SIZE_64

// see TimeTeensy3 example for how to use time library

// use https://github.com/tonton81/FlexCAN_T4 library for receiving CAN data

// look into: https://github.com/greiman/SdFat-beta/blob/master/examples/ExFatLogger/ExFatLogger.ino

// configuration variables
Config_Manager config;
SD_CAN_Logger sd_logger(&config);
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

  if (!config.read_config_file()) Serial.println("Config File read error!");
  config.serial_print_bus_config_str(0);
  config.serial_print_bus_config_str(1);
  config.serial_print_bus_config_str(2);

  sd_logger.max_log_size = config.max_log_size;

  sd_logger.set_next_log_filename();
  Serial.print("Logging on file:");
  char log_name[20];
  sd_logger.get_log_filename(log_name);
  Serial.println(log_name);
  sd_logger.start_log();
  // t2.beginPeriodic(sd_logger.flush_sd_file, 1'000'000); // flush sd file every second

  // setup CANBus
  if (config.can_configs[0].log_enabled){
    Serial.println("Beginning CAN1");
    Can1.begin();
    Can1.setBaudRate(config.can_configs[0].baudrate*1000);
    Can1.setMaxMB(16);
    Can1.enableFIFO();
    Can1.enableFIFOInterrupt();
    Can1.onReceive(can_callback);
  }
  if (config.can_configs[1].log_enabled){
    Serial.println("Beginning CAN2");
    Can2.begin();
    Can2.setBaudRate(config.can_configs[1].baudrate*1000);
    Can2.setMaxMB(16);
    Can2.enableFIFO();
    Can2.enableFIFOInterrupt();
    Can2.onReceive(can_callback);
  }
  if (config.can_configs[2].log_enabled){
    Serial.println("Beginning CAN3");
    Can3.begin();
    Can3.setBaudRate(config.can_configs[2].baudrate*1000);
    Can3.setMaxMB(16);
    Can3.enableFIFO();
    Can3.enableFIFOInterrupt();
    Can3.onReceive(can_callback);
  }
  status = waiting_for_data;
  set_led_from_status(status);
}

void setup() {
  delay(100);
  setup_led();
  t1.beginPeriodic(blink_builtin_led, 100'000); // 100ms blink every 100ms
  Serial.begin(115200); 
  Serial.println("Starting Program");
  
  config.can_configs[0].port = 1;
  config.can_configs[1].port = 2;
  config.can_configs[2].port = 3;
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
  // Can3.events();
  // if (millis () - target_time >= ONE_SECOND_PERIOD)
  // {
  //   target_time += ONE_SECOND_PERIOD ;   // change scheduled time exactly, no slippage will happen
  //   if (status == no_sd){
  //     setup_from_sd_card();
  //   }
  // }

  // sd_logger.set_next_log_filename();
  // char log_name[20];
  // sd_logger.get_log_filename(log_name);
  // Serial.println(log_name);
  // sd_logger.start_log();
  // delay(10);

  check_serial_time();
}
