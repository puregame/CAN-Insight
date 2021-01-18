#include "config.h"
#include "Arduino.h"
#include <FlexCAN_T4.h>
FlexCAN_T4<CAN3, RX_SIZE_16, TX_SIZE_16> Can1; //orig RX_SIZE_256 TX_SIZE_64

// see TimeTeensy3 example for how to use time library

// use https://github.com/tonton81/FlexCAN_T4 library for receiving CAN data

// look into: https://github.com/greiman/SdFat-beta/blob/master/examples/ExFatLogger/ExFatLogger.ino

// *****   Setup buffer/temp values
//                                    1         2         3         4            5
//                          012345678901234567890123456789012345678901234567 8 9 0
const char CAN_String_Init[64] = "      0.000 1 000 Rx d 8 00 00 00 00 00 00 00 00\r\n\0";
char CAN_String_Temp[64] = "      0.000 1 000 Rx d 8 00 00 00 00 00 00 00 00\r\n\0";
char CAN_String_Len = 50;

// ******* Setup timers
#include "TeensyTimerTool.h"
using namespace TeensyTimerTool;
// This does not work when timelib.h is also included !
//// timer for LED blinking
Timer t1; // generate a timer from the pool (Pool: 2xGPT, 16xTMR(QUAD), 20xTCK)
Timer t2; // generate a timer from the pool (Pool: 2xGPT, 16xTMR(QUAD), 20xTCK)

// ***** setup sd card
// setup sd card datalogging
#include <SD.h>
#include <SPI.h>
char log_file_name[LOG_FILE_NAME_LENGTH] = DEFAULT_LOG_FILE_NAME;
const int chipSelect = BUILTIN_SDCARD;
uint16_t log_number = 0;


//***** setup time 
#include <TimeLib.h>
#include <time.h>
time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

// ** begin functions
void blink_builtin_led()
{
    digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));    
}

void get_current_time(char* str_addr){
  // format as ISO datetime like 2021-01-10T20:14:09Z
  time_t seconds = getTeensy3Time();
  strftime(str_addr, 21, "%Y-%m-%dT%H:%M:%SZ", gmtime(&seconds));
}

void serial_print_current_time(){
  char time_buf[21];
  get_current_time(time_buf);
  Serial.println(time_buf);
}

#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1610331025; // Jan 10 2021 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}

void check_serial_time(){
  if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      Serial.print("Set new serial time: ");
      serial_print_current_time();
    }
  }
}

int start_log(){
  File dataFile = SD.open(log_file_name, FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
  // digital clock display of the time
    char time_buf[21];
    get_current_time(time_buf);

    dataFile.println(HEADER_CSV);
  }
  else{
    Serial.println("file not opened!");
    return 0;
  }
  dataFile.close();
  return 1;
}

int read_config_file() {
  // to do: implement reading config file
  // baud, cak, log_std, log_ext,
  return 1;
}

int can_callback() {
  //todo: determine how CAN messages are received

  // filter by extended and standard
  //filter by ID filter
    // id filtering in akpc CAN_Logger uses if ((rxmsg.EID & iFilterMask) != (iFilterValue & iFilterMask)) continue;
  return 1;
}


void write_sd_data(){
  // open the file.
  File dataFile = SD.open(log_file_name, FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
  // digital clock display of the time
    char time_buf[21];
    get_current_time(time_buf);
  
    dataFile.print(time_buf);
    dataFile.println(" -- this is a test");
  }
  else{
    Serial.println("file not opened!");
  }
  dataFile.close();
  // if the file isn't open, pop up an error:
}

void set_next_log_filename(char* in_file){
  uint16_t file_number_to_try = 0;
  // char file_to_try[LOG_FILE_NAME_LENGTH];
  // strcpy(file_to_try, file);
  sprintf(&in_file[LOG_FILE_NUM_POS], "%03d", file_number_to_try);
  in_file[LOG_FILE_DOT_POS] = '.';

  bool next_file_found = false;
  while (!next_file_found){
    if (!SD.exists(in_file)){
      next_file_found = true;
      return;
    }
    file_number_to_try ++;
    sprintf(&in_file[LOG_FILE_NUM_POS], "%03d", file_number_to_try);
    in_file[LOG_FILE_DOT_POS] = '.';
  }
}

void setup() {
  Serial.begin(115200); 
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
  }
  #ifdef DEBUG
  delay(5000); // delay 5 seconds to allow computer to open serial connection
  #endif
  pinMode(LED_BUILTIN, OUTPUT);
  
  Serial.println("Started");
  setSyncProvider(getTeensy3Time);
  
  set_next_log_filename(log_file_name);
  Serial.print("Logging on file:");
  Serial.println(log_file_name);
  start_log();
    
  if (timeStatus()!= timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("Sync with RTC complete");
  }

  //Setup Periodic blink timer 
//  pinMode(LED_BUILTIN,OUTPUT);  
  t1.beginPeriodic(blink_builtin_led, 100'000); // 100ms blink every 100ms
//
  t2.beginPeriodic(write_sd_data, 2000'000); // write to sd card every 2s

  Serial.print("Current Time: ");
  serial_print_current_time();
}

unsigned long target_time = 0L ;
#define ONE_SECOND_PERIOD 1*1000

void loop ()
{
  if (millis () - target_time >= ONE_SECOND_PERIOD)
  {
    target_time += ONE_SECOND_PERIOD ;   // change scheduled time exactly, no slippage will happen
    Serial.println("code running");
  //    write_sd_data();
  //    Serial.print("Current Time: ");
  //    print_current_time();
  //    blink_builtin_led();
  }

  check_serial_time();
}