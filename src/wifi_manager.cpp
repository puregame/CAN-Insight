#include "config.h"
#include "wifi_manager.h"

Wifi_Manager::Wifi_Manager(){
  WiFi.setPins(WIFI_CS_PIN, WIFI_IRQ_PIN, WIFI_RESET_PIN, WIFI_EN_PIN);
  for (int i = 0; i < MAX_SAVED_NETWORK_COUNT; i++){
    wifi_rssi[i] = DEFAULT_NO_NETWORK_RSSI_VALUE;
  }
}

int Wifi_Manager::get_status(){
    return WiFi.status();
}

WiFiClient& Wifi_Manager::get_client(){
  return &wifi_client;
}

void Wifi_Manager::ping_server(char* server_address){
  int pingResult = WiFi.ping(server_address);

  if (pingResult >= 0) {
    Serial.print("SUCCESS! RTT = ");
    Serial.print(pingResult);
    Serial.println(" ms");
  } else {
    Serial.print("FAILED! Error code: ");
    Serial.println(pingResult);
  }
}

bool Wifi_Manager::search_and_connect(){
  // check if shield is present
  // if (WiFi.status() == WL_NO_SHIELD) {
  //   Serial.println("WiFi module not present or not communicating");
  //   // don't continue:
  //   return false;
  // }
  uint8 num_ssid = WiFi.scanNetworks();

  for (uint8 i=0; i < num_ssid; i++){
    if (strcmp(WiFi.SSID(i), "") == 0){
      continue;
    }
    for (uint8 j=0; j < MAX_SAVED_NETWORK_COUNT; j++){
      if (strcmp(WiFi.SSID(i), possible_networks[j].ssid) == 0){
        // future: set RSSI in rssi array to the value of this network

        Serial.print("Found known network, attmpt to connect with ssid: ");
        Serial.println(possible_networks[j].ssid);
        WiFi.begin(possible_networks[j].ssid, possible_networks[j].password);
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("Connection failed");
          print_wifi_status();
          return false;
        }
        #ifdef DEBUG
          Serial.println("Connected to Wifi!");
          print_connection_status();
        #endif
        return true;
      }
    }
  }

  // future: Find the highest value in the RSSI array, attempt to connect to that one
  // if it does not work then try to connect to the second highest and so on

  Serial.println("Did not find any known networks, connecting failed");
  return false;
}

void Wifi_Manager::print_connection_status(){
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

bool Wifi_Manager::set_new_saved_network(char* ssid, char* password){
  if ((strlen(ssid) > M2M_MAX_SSID_LEN) | ((strlen(password) > M2M_MAX_PSK_LEN) | (n_saved_networks == MAX_SAVED_NETWORK_COUNT))){
    return false;
  }
  strcpy(possible_networks[n_saved_networks].ssid, ssid);
  strcpy(possible_networks[n_saved_networks].password, password);
  n_saved_networks++;
  return true;
}

bool Wifi_Manager::set_new_saved_network(Wifi_Network network){
  if (n_saved_networks >= MAX_SAVED_NETWORK_COUNT){
    return false;
  }
  possible_networks[n_saved_networks] = network;
  n_saved_networks++;
  return true;

}

void Wifi_Manager::print_saved_networks(){
  for (int i = 0; i < n_saved_networks; i++){
    Serial.print("Network: ");
    Serial.print(possible_networks[i].ssid);
    Serial.print(", password: ");
    Serial.println(possible_networks[i].password);
  }
}

void Wifi_Manager::print_wifi_status(){
  char status = WiFi.status();
  if (status == WL_CONNECTED){
    Serial.println("status connected");
  }
  if (status == WL_DISCONNECTED){
    Serial.println("status disconnected");
  }
  if (status == WL_CONNECT_FAILED){
    Serial.println("status connect fail");
  }
  if (status == WL_CONNECTION_LOST){
    Serial.println("status connect lost");
  }
}
