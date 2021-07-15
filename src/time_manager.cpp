#include <TimeLib.h>
#include <time.h>
#include "Arduino.h"
#include <SdFat.h>
#include "config.h"

extern SdFs sd;


bool rtc_sync_complete(){
    return (timeStatus() == timeSet);
}

void dateTime(uint16_t* date, uint16_t* time)
{
  *date = FAT_DATE(year(), month(), day());
  *time = FAT_TIME(hour(), minute(), second());
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}

void set_sync_provider_teensy3(){
  setSyncProvider(getTeensy3Time);
}

void set_current_time_in_buffer(char* str_addr){
  // format as ISO datetime like 2021-01-10T20:14:09Z
  time_t seconds = Teensy3Clock.get();
  strftime(str_addr, 21, "%Y-%m-%dT%H:%M:%S.", gmtime(&seconds));
  sprintf(str_addr+strlen(str_addr), "%03dZ", millis()%1000);
}

void serial_print_current_time(){
  char time_buf[40];
  set_current_time_in_buffer(time_buf);
  Serial.println(time_buf);
}


unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 10 2021)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}

bool check_serial_time(){
  if (Serial.available()) {
    time_t t = processSyncMessage();
    if (t != 0) {
      Teensy3Clock.set(t); // set the RTC
      Serial.print("Set new serial time: ");
      serial_print_current_time();
      return true;
    }
  }
}

void read_time_file() {
  Serial.println("Reading Time file");
  if (sd.exists(TIME_FILE_NAME)){
    FsFile time_file = sd.open(TIME_FILE_NAME, O_READ);
    time_file.seek(TIME_HEADER);
    unsigned long pctime = 0L;
    pctime = time_file.parseInt();
    Serial.println(pctime);
    if( pctime > DEFAULT_TIME) { // check the value is a valid time (greater than Jan 10 2021)
      Teensy3Clock.set(pctime);
      Serial.print("Set new time via SD Card: ");
      serial_print_current_time();
    }
    else{
      Serial.println("SD Card time too old, known bad time");
    }
    time_file.close();
    sd.remove(TIME_FILE_NAME);
  }
}

bool check_set_rtc_from_wifi(){
  uint32_t ntp_time = WiFi.getTime();
  if (ntp_time > 0){
    // NTP time set correctly, set this to RTC value
    Teensy3Clock.set(ntp_time);
    #ifdef DEBUG
      Serial.print("Updated RTC to NTP server: ");
      serial_print_current_time();
    #endif
    return true;
  }
  return false;
}
