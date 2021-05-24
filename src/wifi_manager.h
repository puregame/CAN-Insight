#include "datatypes.h"
#include "config.h"
#include <ArduinoJson.h>
#include <WiFi101.h>

#ifndef wifi_manager_
#define wifi_manager_

class Wifi_Manager{
  public:
    Wifi_Manager();
    void check_wifi_networks(Wifi_Network* networks);
    bool is_connected();
    int get_status();
    WiFiClient& get_client();
    void print_wifi_status();
    void print_connection_status();
    bool search_and_connect();
    bool set_new_saved_network(char* ssid, char* pass);
    bool set_new_saved_network(Wifi_Network network);
    void print_saved_networks();
    WiFiClient wifi_client;
  private:
    double wifi_rssi[MAX_SAVED_NETWORK_COUNT];
    byte n_saved_networks = 0;
    Wifi_Network possible_networks[MAX_SAVED_NETWORK_COUNT];       
};

#endif
