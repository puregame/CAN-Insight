#include "Arduino.h"
#include <FlexCAN_T4.h>
#include <SdFat.h>
#include "config.h"
#include "datatypes.h"
#include "rgb_led.h"
#include "time_manager.h"
#include "config_manager.h"
#include "can_log.h"
#include "wifi_manager.h"
#include "data_uploader.h"
#include "TeensyTimerTool.h"

using namespace std;


FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can2;
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> Can3;

// configuration variables
Config_Manager config;
SD_CAN_Logger sd_logger(&config);
System_Status status;

// wifi objects
Wifi_Manager wifi_manager = Wifi_Manager();

// ******* Setup timers
// This does not work when timelib.h is also included !
//// timer for LED blinking
TeensyTimerTool::PeriodicTimer can_log_timer(TeensyTimerTool::PIT);
TeensyTimerTool::PeriodicTimer timer_NTP_check(TeensyTimerTool::TCK);

// setup SD Card
SdFs sd;
FsFile file;

uint16_t log_number = 0;
// ** begin functions

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
  if (!sd.begin(SdioConfig(FIFO_SDIO))) {
    status = no_sd;
    set_led_from_status(status);
    #ifdef DEBUG
      Serial.println("Card failed, or not present");
    #endif
    return;
  }
  read_time_file();

  // insert check size stuff here
  uint32_t freeKB = sd.vol()->freeClusterCount();
  Serial.print("Free Clusters: ");
  Serial.println(freeKB);
  freeKB *= sd.vol()->blocksPerCluster()/2;
  Serial.print("Blocks per Cluster: ");
  Serial.println(sd.vol()->blocksPerCluster());
  Serial.print("Bytes per Cluster: ");
  Serial.println(sd.vol()->bytesPerCluster());
  Serial.print("Free space KB: ");
  Serial.println(freeKB);

  if (!config.read_config_file()) Serial.println("Config File read error!");

  config.serial_print_bus_config_str(0);
  config.serial_print_bus_config_str(1);
  config.serial_print_bus_config_str(2);

  for (int i = 0; i < MAX_SAVED_NETWORK_COUNT; i++){
    wifi_manager.set_new_saved_network(config.wifi_nets[i]);
  }
  #ifdef DEBUG
    wifi_manager.print_saved_networks();
  #endif

  sd_logger.max_log_size = config.max_log_size;

  sd_logger.set_next_log_filename();
  #ifdef DEBUG
    Serial.print("Logging on file:");
    char log_name[20];
    sd_logger.get_log_filename(log_name);
    Serial.println(log_name);
  #endif
  sd_logger.start_log();
  can_log_timer.begin(sd_logger.flush_sd_file, 1s); // flush sd file every second

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
  pinMode(WIFI_WAKE, OUTPUT);
  digitalWrite(WIFI_WAKE, HIGH);
  delay(100);
  setup_led();
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

  // Data Upload
  if (config.wifi_enabled){
    #ifdef DEBUG
      Serial.println("Starting Wifi");
      Serial.println("Searching and connecting to network");
      delay(10);
    #endif
    wifi_manager.search_and_connect();
    DataUploader data_uploader = DataUploader(wifi_manager.get_client(), config.server, config.port, sd_logger.next_log_file_number-1);
    
    #ifdef DEBUG
      Serial.print("next log to upload: ");
      Serial.println(data_uploader.next_log_to_upload);
    #endif
    
    if (wifi_manager.get_status() == WL_CONNECTED){
      #ifdef DEBUG
        Serial.println("Connected to network, trying upload new data");
      #endif
      delay(100); // delay to 

      // set RTC through NTP server
      unsigned long before_set_rtc_time = Teensy3Clock.get();
      if (check_set_rtc_from_wifi()){
        if (Teensy3Clock.get() - before_set_rtc_time > 60){
          #ifdef DEBUG
            Serial.println("Setting RTC from Wifi worked and time delta >60s. restarting log to ensure proper datetime");
          #endif
          // if setting NTP worked then check new vs old time and restart logging to make sure time on the log is correct
          sd_logger.restart_logging();  
        }
        else {
          #ifdef DEBUG
            Serial.println("Not resetting logging, time was close enough");
          #endif
        }
      }

      timer_NTP_check.begin(check_set_rtc_from_wifi, 100s);

      data_uploader.upload_data();
      #ifdef DEBUG
        Serial.println("uploaded data");
      #endif
    }
  }
}

unsigned long target_time = 0L ;
#define ONE_SECOND_PERIOD 1*1000
#define TEN_SECOND_PERIOD 10*1000


void loop ()
{
  if (millis () - target_time >= ONE_SECOND_PERIOD)
  {
    target_time += ONE_SECOND_PERIOD ;   // change scheduled time exactly, no slippage will happen
    if (status == no_sd){
      setup_from_sd_card();
    }
  }
  TeensyTimerTool::tick();
  if (check_serial_time()){
    // time was updated, restart logging
    sd_logger.restart_logging();
  }
}