#include "config.h"
#include <Arduino.h>

void sprintf_num_to_logfile_name(int number_to_try, char* log_name){
  if (number_to_try > 999)
    sprintf(&log_name[LOG_FILE_NUM_POS-1], "%04d", number_to_try);
  else 
    sprintf(&log_name[LOG_FILE_NUM_POS], "%03d", number_to_try);
  log_name[LOG_FILE_DOT_POS] = '.';
}

void blink_builtin_led()
{
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    #ifdef DEBUG
      Serial.print("Setting status LED to: ");
      Serial.println(!digitalRead(LED_BUILTIN));
    #endif
}
