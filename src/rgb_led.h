#include "system.h"
#include "Arduino.h"
#define LED_PWM_OFF 256
#define LED_PWM_LOW 240
#define LED_PWM_BRIGHT 150

void rgb_led_off();
void rgb_led_red_low();
void rgb_led_green_low();
void rgb_led_blue_low();
void rgb_led_red_high();
void rgb_led_green_high();
void rgb_led_blue_high();

void cycle_rgb_led();