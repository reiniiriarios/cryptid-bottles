using namespace std;
#include <WiFiNINA.h>
#include <MQTT.h>

#include "../wifi-config.h"
#include "interwebs.h"

Interwebs::Interwebs() {
  mqttBroker = IPAddress(MQTT_SERVER);
  mqttClient = new MQTTClient(2048);
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
}

void Interwebs::setLED(uint8_t r, uint8_t g, uint8_t b) {
  WiFi.setLEDs(r, g, b);
}

// ------------ CONNECTION ------------

bool Interwebs::connect(loading_callback_t loading_callback) {
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println(F("Communication with WiFi module failed"));
    return false;
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println(F("WiFi firmware upgrade available"));
  }
  loading_callback(STATUS_LOADING);

  if (!wifiInit(loading_callback)) {
    Serial.println(F("Connection failed"));
    loading_callback(STATUS_LOADING_WIFI_ERR);
    return false;
  }
  loading_callback(STATUS_LOADING);

  Serial.print(F("Waiting for connection"));
  for (uint8_t i = 0; i < 5; i++) {
    loading_callback(STATUS_LOADING);
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  loading_callback(STATUS_LOADING);

  printWifiStatus();
  loading_callback(STATUS_LOADING);

  if (!mqttInit(loading_callback)) {
    loading_callback(STATUS_LOADING_MQTT_ERR);
    return false;
  }
  loading_callback(STATUS_LOADING);

  return true;
}

bool Interwebs::wifiInit(loading_callback_t loading_callback) {
  Serial.print(F("Attempting to connect to SSID: "));
  Serial.println(WIFI_SSID);

  status = INTERWEBS_STATUS_WIFI_CONNECTING;
  uint8_t wifiStatus = WL_IDLE_STATUS;
  uint8_t completeAttempts = 5;
  do {
    loading_callback(STATUS_LOADING);
    wifiStatus = WiFiDrv::wifiSetPassphrase(WIFI_SSID, strlen(WIFI_SSID), WIFI_PASS, strlen(WIFI_PASS));
    if (wifiStatus != WL_FAILURE) {
      Serial.print(F("Connecting"));
      uint8_t attempts = 5;
      do {
        loading_callback(STATUS_LOADING);
        Serial.print(".");
        delay(800);
        wifiStatus = WiFiDrv::getConnectionStatus();
      } while ((wifiStatus == WL_IDLE_STATUS || wifiStatus == WL_NO_SSID_AVAIL || wifiStatus == WL_SCAN_COMPLETED) && --attempts > 0);
    }
  } while (wifiStatus != WL_CONNECTED && --completeAttempts > 0);
  loading_callback(STATUS_LOADING);

  if (wifiStatus != WL_CONNECTED) {
    status = INTERWEBS_STATUS_WIFI_ERRORS;
    loading_callback(STATUS_LOADING_WIFI_ERR);
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
    Serial.print(F("Reconnecting WiFi: "));
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
      Serial.println(F("Connection idle..."));
    }
    else if (wifiStatus == WL_SCAN_COMPLETED) {
      // keep waiting...
      Serial.println(F("Connection scan complete..."));
    }
    else if (wifiStatus == WL_NO_SSID_AVAIL) {
      // fail
      status = INTERWEBS_STATUS_WIFI_OFFLINE;
      Serial.println(F("Connection failure, no SSID available"));
    }
    else if (wifiStatus == WL_FAILURE) {
      // fail
      status = INTERWEBS_STATUS_WIFI_OFFLINE;
      Serial.println(F("Connection failure"));
    }
    else {
      // fail (unknown)
      status = INTERWEBS_STATUS_WIFI_OFFLINE;
      Serial.print(F("Connection failure, unknown failure: "));
      Serial.println(String(wifiStatus));
    }
    return false;
  }

  return status == INTERWEBS_STATUS_WIFI_CONNECTED;
}

void Interwebs::setBirthLWTtopic(String topic) {
  birth_lwt_topic = topic;
  if (topic != "") {
    mqttClient->setWill(topic.c_str(), "offline");
  } else {
    mqttClient->clearWill();
  }
}

bool Interwebs::mqttInit(loading_callback_t loading_callback) {
  Serial.print(F("MQTT connecting..."));
  loading_callback(STATUS_LOADING);
  mqttClient->begin(mqttBroker, wifiClient);
  loading_callback(STATUS_LOADING);
  mqttClient->onMessage([&](String &topic, String &payload){
    mqttMessageReceived(topic, payload);
  });

  status = INTERWEBS_STATUS_MQTT_CONNECTING;

  // Connect
  bool connected = false;
  for (int attempts = 5; !connected && attempts >= 0; attempts--) {
    loading_callback(STATUS_LOADING);
    Serial.print(".");
    connected = mqttClient->connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS);
  }
  loading_callback(STATUS_LOADING);
  if (!connected) {
    Serial.println(F("Error connecting to MQTT broker."));
    loading_callback(STATUS_LOADING_MQTT_ERR);
    return false;
  }
  Serial.println();
  status = INTERWEBS_STATUS_MQTT_CONNECTED;

  mqttSubscribe();
  loading_callback(STATUS_LOADING);
  if (birth_lwt_topic != "") {
    mqttPublish(birth_lwt_topic, "online"); // Birth
  }
  loading_callback(STATUS_LOADING);

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
      if (birth_lwt_topic != "") {
        mqttPublish(birth_lwt_topic, "online"); // Birth
      }
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
    Serial.println(F("Reconnecting MQTT..."));
    status = INTERWEBS_STATUS_MQTT_CONNECTING;

    if (!mqttNetClient->connected()) {
      Serial.println(F("Connecting via network client..."));

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
        Serial.println(F("No Socket available"));
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

  // step 2 copied from mqttClient->connect()
  if (status = INTERWEBS_STATUS_MQTT_CONNECTING_2) {
    // check step 1 was successful
    if (!mqttNetClient->connected()) {
      status = INTERWEBS_STATUS_MQTT_OFFLINE;
      mqttClient->*robbed<MQTTClientLastError>::ptr = LWMQTT_NETWORK_FAILED_CONNECT;
      return false;
    }

    Serial.println(F("MQTT connecting to broker..."));
    // prepare options
    lwmqtt_options_t options = lwmqtt_default_options;
    options.keep_alive = 60;
    options.clean_session = true;
    options.client_id = lwmqtt_string(MQTT_CLIENT_ID);
    options.username = lwmqtt_string(MQTT_USER);
    options.password = lwmqtt_string(MQTT_PASS);

    // connect to broker
    // biggest delay here! cannot reasonably recode to flagged loop
    mqttClient->*robbed<MQTTClientLastError>::ptr = lwmqtt_connect(
      &(mqttClient->*robbed<MQTTClientClient>::ptr),
      options,
      mqttClient->*robbed<MQTTClientWill>::ptr,
      &(mqttClient->*robbed<MQTTClientReturnCode>::ptr),
      750 // default = 1000, will err LWMQTT_NETWORK_TIMEOUT if reached
    );
    if (mqttClient->*robbed<MQTTClientLastError>::ptr != LWMQTT_SUCCESS) {
      Serial.println(F("MQTT broker connection failed."));
      mqttClient->*robbed<MQTTClientConnected>::ptr = false;
      // modified mqttNetClient->stop(), without delay
      if (mqttNetClient->*robbed<WiFiClientSock>::ptr != 255) {
        ServerDrv::stopClient(mqttNetClient->*robbed<WiFiClientSock>::ptr);
        status = INTERWEBS_STATUS_MQTT_CLOSING_SOCKET;
      }
      return false;
    }
    Serial.println(F("MQTT connected to broker."));
    status = INTERWEBS_STATUS_MQTT_CONNECTION_SUCCESS;
    mqttClient->*robbed<MQTTClientConnected>::ptr = true;
    // success, but still need subscriptions
    return false;
  }

  if (status == INTERWEBS_STATUS_MQTT_CONNECTED && birth_lwt_topic != "") {
    mqttPublish(birth_lwt_topic, "online"); // Birth
  }
  return status == INTERWEBS_STATUS_MQTT_CONNECTED;
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
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  // print the board's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);
  // print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print(F("Signal strength (RSSI): "));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}

// ------------ MESSAGING ------------

void Interwebs::mqttSendMessage(String topic, String payload) {
  if (wifiIsConnected() && mqttIsConnected()) {
    Serial.print(F("MQTT publishing to "));
    Serial.println(topic);
    if (!mqttClient->publish(topic, payload, true, 1)) {
      Serial.println(F("Error publishing"));
    }
  }
}

bool Interwebs::mqttSubscribe(void) {
  Serial.println(F("MQTT subscribing..."));
  bool success = true;
  for (auto sub : mqttSubs) {
    if (!mqttClient->subscribe(sub.first)) {
      Serial.print(F("Error subscribing to "));
      Serial.println(sub.first);
      success = false;
    }
  }
  if (!success) {
    status = INTERWEBS_STATUS_MQTT_SUBSCRIPTION_FAIL;
    return false;
  }
  status = INTERWEBS_STATUS_MQTT_CONNECTED;
  return true;
}

void Interwebs::onMqtt(String topic, mqttcallback_t callback) {
  mqttSubs[topic] = callback;
}

void Interwebs::mqttMessageReceived(String &topic, String &payload) {
  Serial.print(F("MQTT receipt: "));
  Serial.println(topic + "=" + payload);
  if (mqttSubs.find(topic) == mqttSubs.end()) {
    Serial.print(F("Unrecognized MQTT topic: "));
    Serial.println(topic);
    return;
  }
  mqttSubs[topic](payload);
}

bool Interwebs::mqttPublish(String topic, String payload) {
  if (!mqttClient->publish(topic, payload)) {
    Serial.print(F("Publish error: "));
    Serial.println(String(mqttClient->lastError()));
    return false;
  }
  return true;
}
