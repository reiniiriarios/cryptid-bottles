#include "interwebs.h"

// -------------------------------------- SUBSCRIPTION CLASS ---------------------------------------


MQTTSubscribe::MQTTSubscribe(Adafruit_MQTT *mqttserver, const char *topic, uint8_t qos)
  : Adafruit_MQTT_Subscribe(mqttserver, topic, qos) {};

void MQTTSubscribe::setCallback(mqttcallback_t cb) {
  this->callback_lambda = cb;
}

// ------------------------------------------ MAIN CLASS -------------------------------------------

Interwebs::Interwebs() : Adafruit_MQTT(nullptr, MQTT_PORT, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS) {
  // Configure connection to WiFi board.
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);

  // Create WiFi client.
  this->wifiClient = new WiFiClient();
  this->_sock = &(this->wifiClient->*robbed<WiFiClientSock>::ptr);
  this->mqttSubs = &(this->*robbed<MQTTSubs>::ptr);
}

void Interwebs::setLED(uint8_t r, uint8_t g, uint8_t b) {
  WiFi.setLEDs(r, g, b);
}

// --------------------------------------- CONNECTION LOOPS ----------------------------------------

bool Interwebs::wifiConnectionLoop(void) {
  switch (this->status) {
    case INTERWEBS_STATUS_WIFI_CONNECTED:
      return true;
    case INTERWEBS_STATUS_WIFI_CLOSING_SOCKET:
      this->closeSocket(false);
      return false;
    case INTERWEBS_STATUS_WIFI_ERRORS:
      this->closeConnection(false);
      return false;
    case INTERWEBS_STATUS_INIT:
    case INTERWEBS_STATUS_WIFI_OFFLINE:
      this->wifiSetup();
      return false;
    case INTERWEBS_STATUS_WIFI_READY:
      this->wifiConnect();
      return false;
    default:
      return this->wifiIsConnected();
  }
}

bool Interwebs::mqttConnectionLoop(void) {
  switch (this->status) {
    case INTERWEBS_STATUS_MQTT_CONNECTED:
      return true;
    case INTERWEBS_STATUS_MQTT_CLOSING_SOCKET:
      this->closeSocket(true);
      return false;
    case INTERWEBS_STATUS_MQTT_ERRORS:
      this->closeConnection(true);
      return false;
    case INTERWEBS_STATUS_WIFI_CONNECTED:
    case INTERWEBS_STATUS_MQTT_DISCONNECTED:
    case INTERWEBS_STATUS_MQTT_OFFLINE:
      this->mqttConnect();
      return false;
    case INTERWEBS_STATUS_MQTT_CONNECTING:
      this->waitOnConnection();
      return false;
    case INTERWEBS_STATUS_MQTT_CONNECTION_WAIT:
      this->waitAfterConnection();
      return false;
    case INTERWEBS_STATUS_MQTT_CONNECTION_SUCCESS:
      this->mqttConnectBroker();
      return false;
    case INTERWEBS_STATUS_MQTT_CONNECTED_TO_BROKER:
    case INTERWEBS_STATUS_MQTT_SUBSCRIPTION_FAIL:
      this->mqttSubscribe();
      return false;
    case INTERWEBS_STATUS_MQTT_SUBSCRIBED:
      return this->mqttAnnounce();
    default:
      return this->mqttIsConnected();
  }
}

// -------------------------------- CONNECTION, req implementation ---------------------------------

bool Interwebs::connectServer() {
  // Dummy method, connection happens in loop.
  return false;
}

bool Interwebs::disconnectServer() {
  // Stop connection if connected and return success (stop has no indication of failure).
  if (this->wifiClient->connected()) {
    Serial.println(F("Stopping WiFi client"));
    this->wifiClient->stop();
  }
  return true;
}

int8_t Interwebs::connect() {
  // If not already connected to the server, fail.
  if (!this->wifiClient->connected()) {
    DEBUG_PRINT(F("not connected.."));
    return -1;
  }

  // Construct and send connect packet.
  uint8_t len = (this->*robbed<MQTTConPacket>::ptr)(this->buffer);
  if (!this->sendPacket(this->buffer, len)) {
    DEBUG_PRINT(F("err send packet.."));
    return -1;
  }

  // Read connect response packet and verify it
  len = this->readFullPacket(this->buffer, MAXBUFFERSIZE, CONNECT_TIMEOUT_MS);
  if (len != 4) {
    DEBUG_PRINT(F("err read packet.."));
    return -1;
  }
  if ((this->buffer[0] != (MQTT_CTRL_CONNECTACK << 4)) || (this->buffer[1] != 2)) {
    DEBUG_PRINT(F("err read buf.."));
    return -1;
  }
  if (this->buffer[3] != 0) {
    DEBUG_PRINT(F("buffer ret: "));
    DEBUG_PRINT(this->buffer[3]);
    DEBUG_PRINT(F(".."));
    return this->buffer[3];
  }
  
  return 1;
}

// ---------------------------- CONNECTION LOOP - CLOSE SOCKET, RESTART ----------------------------

bool Interwebs::closeConnection(bool wifi_connected) {
  if (*this->_sock != NO_SOCKET_AVAIL) {
    Serial.println(F("Closing socket..."));
    ServerDrv::stopClient(*this->_sock);
    // In either loop, the next step is to wait for the socket to close.
    this->status = wifi_connected ? INTERWEBS_STATUS_MQTT_CLOSING_SOCKET : INTERWEBS_STATUS_WIFI_CLOSING_SOCKET;
    return false;
  }
  // In WiFi loop, connection is ready to begin.
  // In MQTT loop, connection was closed and needs to reconnect.
  this->status = wifi_connected ? INTERWEBS_STATUS_MQTT_DISCONNECTED : INTERWEBS_STATUS_WIFI_READY;
  return true;
}

bool Interwebs::closeSocket(bool wifi_connected) {
  if (wifiClient->status() != CLOSED) {
    Serial.println(F("Socket closing..."));
    return false;
  }
  WiFiSocketBuffer.close(*_sock);
  *this->_sock = NO_SOCKET_AVAIL;
  Serial.println(F("Socket closed"));
  // In WiFi loop, connection is ready to begin.
  // In MQTT loop, connection was closed and needs to reconnect.
  this->status = wifi_connected ? INTERWEBS_STATUS_MQTT_DISCONNECTED : INTERWEBS_STATUS_WIFI_READY;
  return true;
}

// ------------------------------------ CONNECTION LOOP - WIFI -------------------------------------

bool Interwebs::wifiSetup(void) {
  // If the socket isn't closed, close it and wait for the next loop.
  if (!this->closeConnection(false)) {
    return false;
  }
  // Connect
  Serial.print(F("Connecting WiFi... "));
  Serial.print(WIFI_SSID);
  int8_t ret = WiFiDrv::wifiSetPassphrase(WIFI_SSID, strlen(WIFI_SSID), WIFI_PASS, strlen(WIFI_PASS));
  if (ret == WL_SUCCESS) {
    Serial.println(F("...ready"));
    this->status = INTERWEBS_STATUS_WIFI_READY;
    return true;
  }
  Serial.println(F("...failure"));
  this->status = INTERWEBS_STATUS_WIFI_ERRORS;
  return false;
}

bool Interwebs::wifiConnect(void) {
  int8_t wifiStatus = WiFiDrv::getConnectionStatus();
  switch (wifiStatus) {
    case WL_CONNECTED:
      this->status = INTERWEBS_STATUS_WIFI_CONNECTED;
      Serial.println(F("WiFi connected"));
      return true; // yay
    case WL_IDLE_STATUS:
    case WL_NO_SSID_AVAIL:
    case WL_SCAN_COMPLETED:
      // do nothing, wait...
      return false;
    case WL_FAILURE:
      this->status = INTERWEBS_STATUS_WIFI_OFFLINE;
      Serial.println(F("Connection failure"));
      return false;
    default:
      this->status = INTERWEBS_STATUS_WIFI_OFFLINE;
      Serial.print(F("Connection failure, unknown failure: "));
      Serial.println(String(wifiStatus));
      return false;
  }
}

// ------------------------------------ CONNECTION LOOP - MQTT -------------------------------------

bool Interwebs::mqttConnect(void) {
  // We're reconnecting, so close the old connection first if open.
  if (*this->_sock != NO_SOCKET_AVAIL) {
    if (!this->closeConnection(true)) {
      return false;
    }
  }

  // Get a socket and confirm or restart.
  *this->_sock = ServerDrv::getSocket();
  if (*this->_sock == NO_SOCKET_AVAIL) {
    // failure, flag to start over
    Serial.println(F("No Socket available"));
    this->status = INTERWEBS_STATUS_MQTT_OFFLINE;
    return false;
  }
  Serial.print(F("Connecting on socket "));
  Serial.println(*this->_sock);

  // Connect to server.
  this->status = INTERWEBS_STATUS_MQTT_CONNECTING;
  Serial.println(F("Connecting to server..."));
  ServerDrv::startClient(uint32_t(IPAddress(MQTT_SERVER)), (uint16_t)1883, *this->_sock);
  this->timer = millis();
  return false;
}

bool Interwebs::waitOnConnection(void) {
  if (this->wifiClient->connected()) {
    Serial.print(F("Connected to MQTT server, status: "));
    Serial.println(this->wifiClient->status());
    this->status = INTERWEBS_STATUS_MQTT_CONNECTION_WAIT;
    this->timer = millis(); // start for next wait
    return true;
  }
  // If we've waited long enough, give up and start over.
  if (millis() - this->timer > 4000) {
    Serial.print(F("Connection to MQTT server failed, status: "));
    Serial.println(this->wifiClient->status());
    this->status = INTERWEBS_STATUS_MQTT_OFFLINE;
    this->timer = 0;
    return false;
  }
  // Keep waiting...
  return false;
}

bool Interwebs::waitAfterConnection(void) {
  if (millis() - this->timer > 4000) {
    this->status = INTERWEBS_STATUS_MQTT_CONNECTION_SUCCESS;
    this->timer = 0;
    return true;
  }
  return false;
}

bool Interwebs::mqttConnectBroker(void) {
  Serial.print(F("MQTT connecting to broker..."));
  if (this->connect() != 1) {
    this->attempts++;
    if (this->attempts >= 5 || !this->wifiClient->connected()) {
      this->status = INTERWEBS_STATUS_MQTT_ERRORS;
      this->attempts = 0;
    }
    return false;
  }
  Serial.println(F("success"));
  this->status = INTERWEBS_STATUS_MQTT_CONNECTED_TO_BROKER;
  return false;
}

bool Interwebs::mqttSubscribe(void) {
  Serial.print(F("MQTT subscribing..."));
  for (uint8_t i = 0; i < MAXSUBSCRIPTIONS; i++) {
    MQTTSubscribe* sub = (MQTTSubscribe*)((*this->mqttSubs)[i]);
    if (!sub || sub->topic == nullptr) {
      continue;
    }
    boolean success = false;
    for (uint8_t retry = 0; (retry < 3) && !success; retry++) { // retry until we get a suback
      // Construct and send subscription packet.
      uint8_t len = (this->*robbed<MQTTSubPacket>::ptr)(this->buffer, sub->topic, 0);
      // uint8_t len = this->subscribePacket(this->buffer, sub->topic, 0);
      if (!this->sendPacket(this->buffer, len)) {
        Serial.println(F("error sending packet"));
        this->status = INTERWEBS_STATUS_MQTT_SUBSCRIPTION_FAIL;
        return false;
      }
      if (this->processPacketsUntil(this->buffer, MQTT_CTRL_SUBACK, SUBACK_TIMEOUT_MS)) {
        success = true;
        break;
      }
    }
    if (!success) {
      Serial.println(F("error"));
      this->status = INTERWEBS_STATUS_MQTT_SUBSCRIPTION_FAIL;
      return false;
    }
  }
  Serial.println(F("success"));
  this->status = INTERWEBS_STATUS_MQTT_SUBSCRIBED;
  return true;
}

bool Interwebs::mqttAnnounce(void) {
  if (this->birth_msg.first != "") {
    if (!this->mqttPublish(this->birth_msg.first, this->birth_msg.second)) {
      return false;
    }
  }
  this->status = INTERWEBS_STATUS_MQTT_CONNECTED;
  return true;
}

// -------------------------------------- MAIN LOOP - RECEIVE --------------------------------------

bool Interwebs::mqttLoop(void) {
  // If WiFi isn't connected, and we know it, skip this loop.
  if (!this->mqttIsConnected()) {
    return false;
  }
  // Try to read message in queue first.
  if (!this->processSubscriptionQueue()) {
    // If nothing to read, try processing a packet instead.
    this->processIncomingSubscriptions();
  }
  return true;
}

// --------------------------------------- CONNECTION STATUS ---------------------------------------

bool Interwebs::verifyConnection(void) {
  Serial.print(F("Verifying connection..."));
  if (!this->ping()) {
    Serial.println(F("ping failed"));
    this->disconnect();
    this->status = INTERWEBS_STATUS_MQTT_OFFLINE;
    return false;
  }
  Serial.println(F("online"));
  return true;
}

bool Interwebs::connected(void) {
  return this->status == INTERWEBS_STATUS_MQTT_CONNECTED;
}

bool Interwebs::wifiIsConnected(void) {
  return this->status >= 10;
}

bool Interwebs::mqttIsConnected(void) {
  return this->status >= 30;
}

void Interwebs::printWifiStatus(void) {
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());
  Serial.print(F("Signal strength (RSSI): "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));
}

// ------------------------------------------- MESSAGING -------------------------------------------

void Interwebs::setBirth(String topic, String payload) {
  this->birth_msg = { topic, payload };
}

void Interwebs::onMqtt(const char* topic, mqttcallback_t callback) {
  MQTTSubscribe* sub = new MQTTSubscribe(this, topic);
  sub->setCallback(callback);
  this->subscribe(sub);
}

void Interwebs::mqttSendMessage(String topic, String payload) {
  if (!this->connected()) {
    return;
  }
  Serial.print(F("MQTT publishing to "));
  Serial.println(topic);
  if (!this->mqttPublish(topic, payload)) {
    Serial.println(F("Error publishing"));
  }
}

bool Interwebs::mqttPublish(String topic, String payload, bool retain, uint8_t qos) {
  const char *data = payload.c_str();
  return this->publish(topic.c_str(), (uint8_t *)(data), strlen(data), qos, retain);
}

bool Interwebs::processSubscriptionQueue(void) {
  for (uint8_t i = 0; i < MAXSUBSCRIPTIONS; i++) {
    MQTTSubscribe* sub = (MQTTSubscribe*)((*this->mqttSubs)[i]);
    if (sub && sub->new_message) {
      sub->callback_lambda((char *)sub->lastread, sub->datalen);
      sub->new_message = false;
      return true; // only read one
    }
  }
  return false; // none read
}

// -------------------------------------- PACKET PROCESSING ----------------------------------------

void Interwebs::processIncomingSubscriptions(void) {
  // If data is available to be read, read and process a single packet.
  if (this->wifiClient->available()) {
    uint16_t len = this->readFullPacket(this->buffer, MAXBUFFERSIZE, READ_PACKET_TIMEOUT);
    this->handleSubscriptionPacket(len);
  }
}

uint16_t Interwebs::readPacket(uint8_t *buf, uint16_t maxlen, int16_t timeout) {
  if (maxlen == 0) return 0; // handle zero-length packets
  // Read data until either the connection is closed, or the idle timeout is reached.
  uint16_t len = 0;
  int16_t t = timeout;
  while (this->wifiClient->connected() && (timeout >= 0)) {
    while (this->wifiClient->available()) {
      char c = this->wifiClient->read();
      timeout = t; // reset the timeout
      buf[len] = c;
      len++;
      if (len == maxlen) { // we read all we want, bail
        DEBUG_PRINT(F("Read data:\t"));
        DEBUG_PRINTBUFFER(buf, len);
        return len;
      }
    }
    timeout -= MQTT_CLIENT_READINTERVAL_MS;
    delay(MQTT_CLIENT_READINTERVAL_MS);
  }
  return len;
}

bool Interwebs::sendPacket(uint8_t *buf, uint16_t len) {
  uint16_t ret = 0;
  uint16_t offset = 0;
  while (len > 0) {
    if (!this->wifiClient->connected()) {
      this->status = INTERWEBS_STATUS_MQTT_OFFLINE;
      DEBUG_PRINTLN(F("Connection failed!"));
      return false;
    }
    // send 250 bytes at most at a time, can adjust this later based on Client
    uint16_t sendlen = len > 250 ? 250 : len;
    ret = this->wifiClient->write(buf + offset, sendlen);
    DEBUG_PRINT(F("Client sendPacket returned: "));
    DEBUG_PRINTLN(ret);
    len -= ret;
    offset += ret;

    if (ret != sendlen) {
      DEBUG_PRINTLN("Failed to send packet.");
      return false;
    }
  }
  return true;
}
