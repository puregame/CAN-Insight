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
  t2.beginPeriodic(sd_logger.flush_sd_file, 1'000'000); // flush sd file every second

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

//** wifi stuff
#include <SPI.h>
#include <ArduinoHttpClient.h>
#include <WiFi101.h>
#include "wifi_secrets.h" 
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int wifi_status = WL_IDLE_STATUS;
char server[] = "192.168.1.173";
int port = 5000;
void printWiFiStatus(); 
WiFiClient wifi;
HttpClient http_client = HttpClient(wifi, server, port);


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

  // WIFI stuff below
  Serial.println("Starting Wifi");
  WiFi.setPins(WIFI_CS_PIN, WIFI_IRQ_PIN, WIFI_RESET_PIN, WIFI_EN_PIN);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }
  while (wifi_status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    wifi_status = WiFi.begin(ssid, pass);

    // wait 1 second for connection:
    delay(1000);
  }
  Serial.println("Connected to wifi");
  printWiFiStatus();
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

  // http_client.post("/", "text/plain", "this is a body");// read the status code and body of the response
  // int statusCode = http_client.responseStatusCode();
  // String response = http_client.responseBody();
  String request_uri = "/data_file/?unit_type=";
  request_uri = request_uri+"test"+"&unit_number=" + "test" + "&log_time=" + "2021-03-06T12-46-01-466Z";
  http_client.beginRequest();
  http_client.get(request_uri);
  http_client.endRequest();
  
  if (http_client.responseStatusCode() == 404) {
    Serial.println("Got 404 status code, will upload file");
    Serial.println("Connecting to server...");
      if (wifi.connect(server, port)) {
        Serial.println("- connected");
    #define LOG_NAME "CAN_008.LOG"
        wifi.print("POST /data_file/?log_name=");
        wifi.print(LOG_NAME);
        wifi.println(" HTTP/1.1");
        wifi.println("Host: 192.168.1.173");
        wifi.println("User-Agent: Arduino/1.0");
        wifi.println("Connection: close");
    //    wifi.println("Content-Type: application/x-www-form-urlencoded");
        wifi.println("Content-Type: text/plain; charset=UTF-8");
        wifi.print("Content-Length: ");
        
        Serial.println("reading Config file");
        File send_file = SD.open(LOG_NAME, FILE_READ);
        wifi.println(send_file.size());
        wifi.println();
        for (int i = 0; i < send_file.size(); i++){
          wifi.write(send_file.read());
        }
        Serial.println("- done");
      }

  }
  else
    Serial.println("Got not 404 status code, will not upload file");
    
  Serial.println("Wait 1 seconds");
  delay(1000);

  check_serial_time();
}


void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}