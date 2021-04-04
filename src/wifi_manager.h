#include "datatypes.h"
#include "config.h"
#include <ArduinoJson.h>
#include <WiFi101.h>

#ifndef wifi_manager_
#define wifi_manager_
class Wifi_Network{
  public:
    char ssid[M2M_MAX_SSID_LEN];
    char password[M2M_MAX_PSK_LEN];
};

class Wifi_Manager{
  public:
    Wifi_Manager();
    void check_wifi_netowrks(Wifi_Network* networks);
    bool is_connected();
    int get_status();
    WiFiClient& get_client();
    void print_wifi_status();
    void print_connection_status();
    bool search_and_connect();
    bool set_new_saved_network(char* ssid, char* pass);
    WiFiClient wifi_client;
  private:
    byte n_saved_networks = 0;
    Wifi_Network possible_networks[MAX_SAVED_NETWORK_COUNT];       
};
#endif
