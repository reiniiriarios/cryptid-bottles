#include <WiFiNINA.h>
#include <MQTT.h>

#include "../wifi-config.h"
#include "interwebs.h"

Interwebs::Interwebs(bool *PIXELS_ON_p) {
  PIXELS_ON = PIXELS_ON_p;
  mqttBroker = IPAddress(MQTT_SERVER);
  mqttClient = new MQTTClient(1024);
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
}

// ------------ CONNECTION ------------

bool Interwebs::connect(void) {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed");
    return false;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("WiFi firmware upgrade available");
  }

  if (!wifiInit()) {
    Serial.println("Connection failed");
    return false;
  }

  Serial.print("Waiting for connection");
  for (uint8_t i = 0; i < 5; i++) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  printWifiStatus();

  if (!mqttInit()) {
    return false;
  }

  return true;
}

bool Interwebs::wifiInit(void) {
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(WIFI_SSID);

  status = INTERWEBS_STATUS_WIFI_CONNECTING;
	uint8_t wifiStatus = WL_IDLE_STATUS;
  uint8_t completeAttempts = 5;
  do {
    wifiStatus = WiFiDrv::wifiSetPassphrase(WIFI_SSID, strlen(WIFI_SSID), WIFI_PASS, strlen(WIFI_PASS));
    if (wifiStatus != WL_FAILURE) {
      Serial.print("Connecting");
    	uint8_t attempts = 5;
      do {
        Serial.print(".");
        delay(2000);
        wifiStatus = WiFiDrv::getConnectionStatus();
      } while ((wifiStatus == WL_IDLE_STATUS || wifiStatus == WL_NO_SSID_AVAIL || wifiStatus == WL_SCAN_COMPLETED) && --attempts > 0);
    }
  } while (wifiStatus != WL_CONNECTED && --completeAttempts > 0);

  if (wifiStatus != WL_CONNECTED) {
    status = INTERWEBS_STATUS_WIFI_ERRORS;
    return false;
  }

  status = INTERWEBS_STATUS_WIFI_CONNECTED;
  return true;
}

bool Interwebs::wifiReconnect(void) {
  if (status == INTERWEBS_STATUS_WIFI_CONNECTED || status == INTERWEBS_STATUS_MQTT_CONNECTED) {
    status = INTERWEBS_STATUS_WIFI_OFFLINE;
  }

  // step 1, initiate connection
  if (status == INTERWEBS_STATUS_WIFI_OFFLINE) {
    Serial.print("Reconnecting WiFi: ");
    Serial.println(WIFI_SSID);
    uint8_t wifiStatus = WiFiDrv::wifiSetPassphrase(WIFI_SSID, strlen(WIFI_SSID), WIFI_PASS, strlen(WIFI_PASS));
    status = INTERWEBS_STATUS_WIFI_CONNECTING;
    return false;
  }

  // step 2, check connection
  if (status == INTERWEBS_STATUS_WIFI_CONNECTING) {
    uint8_t wifiStatus = WiFiDrv::getConnectionStatus();
    if (wifiStatus == WL_CONNECTED) {
      // yay!
      status = INTERWEBS_STATUS_WIFI_CONNECTED;
      return true;
    }
    else if (wifiStatus == WL_IDLE_STATUS) {
      // keep waiting...
      Serial.println("Connection idle...");
    }
    else if (wifiStatus == WL_SCAN_COMPLETED) {
      // keep waiting...
      Serial.println("Connection scan complete...");
    }
    else if (wifiStatus == WL_NO_SSID_AVAIL) {
      // fail
      status = INTERWEBS_STATUS_WIFI_OFFLINE;
      Serial.println("Connection failure, no SSID available");
    }
    else if (wifiStatus == WL_FAILURE) {
      // fail
      status = INTERWEBS_STATUS_WIFI_OFFLINE;
      Serial.println("Connection failure");
    }
    else {
      // fail (unknown)
      status = INTERWEBS_STATUS_WIFI_OFFLINE;
      Serial.println("Connection failure, unknown failure (" + String(wifiStatus) + ")");
    }
    return false;
  }

  return status == INTERWEBS_STATUS_WIFI_CONNECTED;
}

bool Interwebs::mqttInit(void) {
  Serial.print("MQTT connecting...");
  mqttClient->begin(mqttBroker, wifiClient);
  mqttClient->onMessage([&](String &topic, String &payload){
    mqttMessageReceived(topic, payload);
  });

  status = INTERWEBS_STATUS_MQTT_CONNECTING;

  // Connect
  bool connected = false;
  for (int attempts = 5; !connected && attempts >= 0; attempts--) {
    Serial.print(".");
    connected = mqttClient->connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
  }
  if (!connected) {
    Serial.println("Error connecting to MQTT broker.");
    return false;
  }
  Serial.println();
  status = INTERWEBS_STATUS_MQTT_CONNECTED;

  mqttSubscribe();
  mqttSendDiscovery();

  return true;
}

bool Interwebs::mqttReconnect(void) {
  if (status == INTERWEBS_STATUS_MQTT_CONNECTED) {
    status = INTERWEBS_STATUS_MQTT_OFFLINE;
  }

  if (status == INTERWEBS_STATUS_MQTT_SUBSCRIPTION_FAIL || status == INTERWEBS_STATUS_MQTT_CONNECTION_SUCCESS) {
    if (!mqttClient->connected()) {
      status = INTERWEBS_STATUS_MQTT_OFFLINE;
    }
    else {
      mqttSubscribe();
      return status == INTERWEBS_STATUS_MQTT_CONNECTED;
    }
  }

  // mqttClient->netClient
  WiFiClient* mqttNetClient = (WiFiClient*)(mqttClient->*robbed<MQTTClientNetClient>::ptr);

  // step 1b copied from WiFiClient::stop()
  if (status == INTERWEBS_STATUS_MQTT_CLOSING_SOCKET) {
    if (mqttNetClient->status() != CLOSED) {
      // if not closed yet, keep waiting until the next loop
      return false;
    }
    WiFiSocketBuffer.close(mqttNetClient->*robbed<WiFiClientSock>::ptr);
    // _sock
    mqttNetClient->*robbed<WiFiClientSock>::ptr = NO_SOCKET_AVAIL;
    status = INTERWEBS_STATUS_MQTT_CONNECTING;
  }

  // step 1 copied from WiFiClient::connect()
  if (status == INTERWEBS_STATUS_MQTT_OFFLINE || status == INTERWEBS_STATUS_MQTT_CONNECTING) {
    Serial.println("Reconnecting MQTT...");
    status = INTERWEBS_STATUS_MQTT_CONNECTING;

    if (!(mqttClient->*robbed<MQTTClientNetClient>::ptr)->connected()) {
      Serial.println("Connecting via network client...");

      // step 1a copied from WiFiClient::stop()
      // If the socket isn't closed, close it and wait for the next loop.
      if (mqttNetClient->*robbed<WiFiClientSock>::ptr != NO_SOCKET_AVAIL) {
        ServerDrv::stopClient(mqttNetClient->*robbed<WiFiClientSock>::ptr);
        status = INTERWEBS_STATUS_MQTT_CLOSING_SOCKET;
        return false;
      }

      mqttNetClient->*robbed<WiFiClientSock>::ptr = ServerDrv::getSocket();
      if (mqttNetClient->*robbed<WiFiClientSock>::ptr == NO_SOCKET_AVAIL) {
        // failure, flag to start over
      	Serial.println("No Socket available");
        mqttClient->*robbed<MQTTClientLastError>::ptr = LWMQTT_NETWORK_FAILED_CONNECT;
        status = INTERWEBS_STATUS_MQTT_OFFLINE;
        return false;
      }

    	ServerDrv::startClient(uint32_t(mqttBroker), (uint16_t)1883, mqttNetClient->*robbed<WiFiClientSock>::ptr);
    }
    // step complete, break
    status = INTERWEBS_STATUS_MQTT_CONNECTING_2;
    return false;
  }

  // step 2 copied from mqttClient->:connect()
  if (status = INTERWEBS_STATUS_MQTT_CONNECTING_2) {
    // check step 1 was successful
  	if (!(mqttClient->*robbed<MQTTClientNetClient>::ptr)->connected()) {
      status = INTERWEBS_STATUS_MQTT_OFFLINE;
      mqttClient->*robbed<MQTTClientLastError>::ptr = LWMQTT_NETWORK_FAILED_CONNECT;
      return false;
  	}

    Serial.println("MQTT connecting to broker...");
    // prepare options
    lwmqtt_options_t options = lwmqtt_default_options;
    options.keep_alive = 60;
    options.clean_session = true;
    options.client_id = lwmqtt_string(MQTT_CLIENT_ID);
    options.username = lwmqtt_string(MQTT_USER);
    options.password = lwmqtt_string(MQTT_PASS);

    // connect to broker
    mqttClient->*robbed<MQTTClientLastError>::ptr = lwmqtt_connect(
      &(mqttClient->*robbed<MQTTClientClient>::ptr),
      options,
      mqttClient->*robbed<MQTTClientWill>::ptr,
      &(mqttClient->*robbed<MQTTClientReturnCode>::ptr),
      1000
    );
    if (mqttClient->*robbed<MQTTClientLastError>::ptr != LWMQTT_SUCCESS) {
      Serial.println("MQTT broker connection failed.");
      mqttClient->*robbed<MQTTClientConnected>::ptr = false;
      (mqttClient->*robbed<MQTTClientNetClient>::ptr)->stop();

      return false;
    }
    Serial.println("MQTT connected to broker.");
    status = INTERWEBS_STATUS_MQTT_CONNECTION_SUCCESS;
    mqttClient->*robbed<MQTTClientConnected>::ptr = true;
    // success, but still need subscriptions
    return false;
  }

  return status == INTERWEBS_STATUS_MQTT_CONNECTED;
}

bool Interwebs::verifyConnection() {
  if (!wifiIsConnected()) {
    status = INTERWEBS_STATUS_WIFI_OFFLINE;
    Serial.println("WiFi disconnected...");
    if (!wifiReconnect()) {
      Serial.println("Error reconnecting WiFi");
      return false;
    }
  }
  if (!mqttIsConnected()) {
    Serial.println("MQTT disconnected...");
    if (!mqttReconnect()) {
      mqttSendDiscovery();
      return false;
    }
  }

  return true;
}

void Interwebs::mqttLoop(void) {
  mqttClient->loop();
}

bool Interwebs::mqttIsConnected(void) {
  return mqttClient->connected() && status == INTERWEBS_STATUS_MQTT_CONNECTED;
}

bool Interwebs::wifiIsConnected(void) {
  return WiFiDrv::getConnectionStatus() == WL_CONNECTED;
}

void Interwebs::printWifiStatus(void) {
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  // print the board's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// ------------ MESSAGING ------------

void Interwebs::mqttSendMessage(String topic, String payload) {
  if (verifyConnection()) {
    Serial.println("MQTT publishing to " + topic);
    if (!mqttClient->publish("cryptid/" + topic, payload)) {
      Serial.println("Error publishing");
    }
  }
}

bool Interwebs::mqttSubscribe(void) {
  Serial.print("MQTT subscribing...");
  bool success = true;

  // HASS
  if (!mqttClient->subscribe("homeassistant/status")) {
    Serial.println("Error subscribing to homeassistant/status");
    success = false;
  }

  // OPERATIONS
  if (!mqttClient->subscribe("cryptid/bottles/set")) {
    Serial.println("Error subscribing to cryptid/bottles/set");
    success = false;
  }

  // done
  if (!success) {
    status = INTERWEBS_STATUS_MQTT_SUBSCRIPTION_FAIL;
    Serial.println("failed to subscribe to all topics.");
    return false;
  }
  status = INTERWEBS_STATUS_MQTT_CONNECTED;
  Serial.println("success.");
  return true;
}

void Interwebs::mqttMessageReceived(String &topic, String &payload) {
  Serial.println("MQTT receipt: " + topic + " = " + payload);

  // HASS
  if (topic == "homeassistant/status") {
    if (payload == "online") {
      mqttSendDiscovery();
    }
    return;
  }

  // OPERATIONS
  if (topic == "cryptid/bottles/set") {
    if (payload == "on" || payload == "ON" || payload.toInt() == 1) {
      *PIXELS_ON = true;
    }
    else if (payload == "off" || payload == "OFF" || payload.toInt() == 0) {
      *PIXELS_ON = false;
    }
    return;
  }

  Serial.println("Unrecognized MQTT topic: " + topic);
}

// ------------ DISCOVERY ------------
// https://www.home-assistant.io/integrations/mqtt#discovery-messages
// <discovery_prefix>/<component>/[<node_id>/]<object_id>/config

bool Interwebs::mqttSendDiscovery(void) {
  String topic = "homeassistant/switch/display/"+String(MQTT_CLIENT_ID)+"/config";
  String payload = "{";
  payload += "\"name\":\"display\",";
  payload += "\"state_topic\":\"cryptid/bottles/state\",";
  payload += "\"command_topic\":\"cryptid/bottles/set\",";
  payload += "\"unique_id\":\""+String(MQTT_CLIENT_ID)+"Set\",";
  payload += "\"device\":{";
  payload += "\"identifiers\":[\""+String(MQTT_CLIENT_ID)+"\"],";
  payload += "\"name\":\"Cryptid Bottles\"";
  payload += "}}";

  if (!mqttClient->publish(topic, payload)) {
    Serial.println("Error publishing discovery for on/off toggle.");
    Serial.println("Error: " + String(mqttClient->lastError()));
    return false;
  }
  return true;
}
