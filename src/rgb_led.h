#include "system.h"
#include "Arduino.h"
#include "datatypes.h"
#define LED_PWM_OFF 256
#define LED_PWM_LOW 240
#define LED_PWM_BRIGHT 150

void set_led_from_status(System_Status status);
void setup_led();
void cycle_rgb_led();