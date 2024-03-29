#include "system.h"
#include "rgb_led.h"
#include "datatypes.h"

byte rgb_status = 0;

void rgb_led_off() {
  analogWrite(LED_RED_PIN, LED_PWM_OFF);
  analogWrite(LED_GREEN_PIN, LED_PWM_OFF);
  analogWrite(LED_BLUE_PIN, LED_PWM_OFF);
}
void rgb_led_red_low() {
  rgb_led_off();
  analogWrite(LED_RED_PIN, LED_PWM_LOW);
}
void rgb_led_orange_low() {
  rgb_led_off();
  analogWrite(LED_RED_PIN, LED_PWM_BRIGHT+30);
  analogWrite(LED_GREEN_PIN, LED_PWM_LOW);
}
void rgb_led_green_low() {
  rgb_led_off();
  analogWrite(LED_GREEN_PIN, LED_PWM_LOW);
}
void rgb_led_blue_low() {
  rgb_led_off();
  analogWrite(LED_BLUE_PIN, LED_PWM_LOW);
}
void rgb_led_red_high() {
  rgb_led_off();
  analogWrite(LED_RED_PIN, LED_PWM_BRIGHT);
}
void rgb_led_green_high() {
  rgb_led_off();
  analogWrite(LED_GREEN_PIN, LED_PWM_BRIGHT);
}
void rgb_led_blue_high() {
  rgb_led_off();
  analogWrite(LED_BLUE_PIN, LED_PWM_BRIGHT);
}

void set_led_from_status(System_Status status){
  switch(status){
    case boot: rgb_led_orange_low(); break;
    case no_sd: rgb_led_red_low(); break;
    case waiting_for_data: rgb_led_green_low(); break;
    case writing_sd: rgb_led_blue_low(); break;
  }
}

void setup_led(){
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);
  set_led_from_status(boot);
}

void cycle_rgb_led(){
  switch (rgb_status) {
    case 0:
      rgb_led_off();
      break;
    case 1:
      rgb_led_red_low();
      break;
    case 2:
      rgb_led_green_low();
      break;
    case 3:
      rgb_led_blue_low();
      break;
    case 4:
      rgb_led_red_high();
      break;
    case 5:
      rgb_led_green_high();
      break;
    case 6:
      rgb_led_blue_high();
      break;
  }
  rgb_status++;
  if (rgb_status>6)
    rgb_status=0;
}
