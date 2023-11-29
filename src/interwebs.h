#ifndef CRYPTID_INTERWEBS_H
#define CRYPTID_INTERWEBS_H

#include <map>
using namespace std;

#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#include <utility/server_drv.h>
#include <utility/WiFiSocketBuffer.h>
#include <Adafruit_MQTT.h>

#include "def.h"
#include "../wifi-config.h"

// --------------------------------------------- DEFS ----------------------------------------------

// How long to delay waiting for new data to be available in readPacket.
#define MQTT_CLIENT_READINTERVAL_MS 10

// -------------------------------------------- ROBBERY --------------------------------------------
// Rob a pointer to a private property of an object.

// Struct to hold the stolen pointer (ptr).
template<typename Tag>
struct robbed {
  typedef typename Tag::type type;
  static type ptr;
};

template<typename Tag>
typename robbed<Tag>::type robbed<Tag>::ptr;

// Struct to rob a pointer.
template<typename Tag, typename Tag::type p>
struct rob : robbed<Tag> {
  struct filler {
    filler() { robbed<Tag>::ptr = p; }
  };
  static filler filler_obj;
};

template<typename Tag, typename Tag::type p>
typename rob<Tag, p>::filler rob<Tag, p>::filler_obj;

// (uint8_t) &WiFiClient::_sock
// wifiClientObj->_sock <=> &(wifiClientObj->*robbed<WiFiClientSock>::ptr);
struct WiFiClientSock { typedef uint8_t WiFiClient::*type; };
template class rob<WiFiClientSock, &WiFiClient::_sock>;

// &Adafruit_MQTT::connectPacket(uint8_t *packet)
// (mqtt->*robbed<MQTTConPacket>::ptr)(...);
struct MQTTConPacket { typedef uint8_t(Adafruit_MQTT::*type)(uint8_t*); };
template class rob<MQTTConPacket, &Adafruit_MQTT::connectPacket>;

// &Adafruit_MQTT::subscribePacket(uint8_t *packet, const char *topic, uint8_t qos)
// (mqtt->*robbed<MQTTSubPacket>::ptr)(...);
struct MQTTSubPacket { typedef uint8_t(Adafruit_MQTT::*type)(uint8_t*, const char*, uint8_t); };
template class rob<MQTTSubPacket, &Adafruit_MQTT::subscribePacket>;

// -------------------------------------------- TYPEDEF --------------------------------------------

/**
 * @brief Interwebs connection status.
 */
typedef enum {
  // Startup
  INTERWEBS_STATUS_INIT = 0,
  // WiFi
  INTERWEBS_STATUS_WIFI_READY = 1,
  INTERWEBS_STATUS_WIFI_OFFLINE = 2,
  INTERWEBS_STATUS_WIFI_ERRORS = 3,
  INTERWEBS_STATUS_WIFI_CLOSING_SOCKET = 4,
  // < 10 = WiFi offline
  INTERWEBS_STATUS_WIFI_CONNECTED = 10,
  // MQTT
  INTERWEBS_STATUS_MQTT_CONNECTING = 20,
  INTERWEBS_STATUS_MQTT_CONNECTION_WAIT = 29,
  INTERWEBS_STATUS_MQTT_CONNECTION_SUCCESS = 23,
  INTERWEBS_STATUS_MQTT_CONNECTED_TO_BROKER = 28,
  INTERWEBS_STATUS_MQTT_CLOSING_SOCKET = 21,
  INTERWEBS_STATUS_MQTT_SUBSCRIBED = 27,
  INTERWEBS_STATUS_MQTT_SUBSCRIPTION_FAIL = 25,
  INTERWEBS_STATUS_MQTT_DISCONNECTED = 22,
  INTERWEBS_STATUS_MQTT_OFFLINE = 24,
  INTERWEBS_STATUS_MQTT_ERRORS = 26,
  // < 30 = MQTT offline
  INTERWEBS_STATUS_MQTT_CONNECTED = 30,
} interwebs_status_t;

/**
 * @brief MQTT subscription callback function.
 */
typedef std::function<void(char*,uint16_t)> mqttcallback_t;

// -------------------------------------- SUBSCRIPTION CLASS ---------------------------------------

/**
 * @brief Subscription class.
 *        Extends base class in order to add support for lambda callbacks.
 */
class MQTTSubscribe : public Adafruit_MQTT_Subscribe {
  public:
    /**
     * @brief Constructor
     * 
     * @param mqttserver pointer to Interwebs
     * @param topic 
     * @param qos
     */
    MQTTSubscribe(Adafruit_MQTT *mqttserver, const char *topic, uint8_t qos = 0);

    /**
     * @brief Set a callback for the subscription.
     * 
     * @param callb Lambda-compatible callback.
     */
    void setCallback(mqttcallback_t callb);

    /**
     * @brief Lambda-compatible callback function.
     */
    mqttcallback_t callback_lambda;
};

// ------------------------------------------ MAIN CLASS -------------------------------------------

/**
 * @brief This class manages WiFi connection and MQTT broker connection as well
 *        as handles MQTT subscription callbacks.
 */
class Interwebs : public Adafruit_MQTT {
  public:
    /**
     * @brief Construct a new Interwebs object.
     */
    Interwebs();

    /**
     * @brief Set the Airlift onboard LED to a specific color.
     * 
     * @param r 
     * @param g 
     * @param b 
     */
    void setLED(uint8_t r, uint8_t g, uint8_t b);

    // -------------------------- CONNECTION LOOP - CLOSE SOCKET, RESTART --------------------------

    /**
     * @brief Reconnect to WiFi. This method operates step-by-step, continuing each call.
     * 
     * @return connected
     */
    bool wifiConnectionLoop(void);

    /**
     * @brief Reconnect to MQTT broker. This method operates step-by-step, continuing each call.
     *
     * @return connected
     */
    bool mqttConnectionLoop(void);

    // ------------------------------------- CONNECTION STATUS -------------------------------------

    /**
     * @brief Return status without querying anything new.
     *
     * @return connected
     */
    bool connected(void) override;

    /**
     * @brief Return WiFi status without querying anything new.
     *
     * @return connected
     */
    bool wifiIsConnected(void);

    /**
     * @brief Return MQTT status without querying anything new.
     *
     * @return connected
     */
    bool mqttIsConnected(void);

    /**
     * @brief Verify MQTT WiFi connection is stable by querying chip over SPI.
     *
     * @return connected
     */
    bool verifyConnection(void);

    /**
     * @brief Print WiFi status over the Serial connection.
     */
    void printWifiStatus(void);

    // ------------------------------------ MAIN LOOP - RECEIVE ------------------------------------

    /**
     * @brief Main MQTT client loop. Run on main loop.
     *
     * @return success
     */
    bool mqttLoop(void);

    // -------------------------------------------- SET --------------------------------------------

    /**
     * @brief Set the birth topic.
     * 
     * @param topic
     * @param payload
     */
    void setBirth(String topic, String payload);

    // ----------------------------------------- MESSAGING -----------------------------------------

    /**
     * @brief MQTT hook.
     *
     * @param topic
     * @param callback
     */
    void onMqtt(String topic, mqttcallback_t callback);

    /**
     * @brief Process subscription packet.
     *        Override from base class to allow for lambda callbacks.
     * 
     * @param sub
     */
    void processSubscriptionPacket(MQTTSubscribe *sub);

    /**
     * @brief Send MQTT message. Verifies connection before sending.
     *
     * @param topic
     * @param payload
     */
    void mqttSendMessage(String topic, String payload);

    /**
     * @brief Publish a MQTT message.
     * 
     * @param topic
     * @param payload
     * @param retain
     * @param qos
     * @return success
     */
    bool mqttPublish(String topic, String payload, bool retain = false, uint8_t qos = 0);

  private:
    /**
     * @brief Current status of interwebs connections.
     */
    int status = INTERWEBS_STATUS_INIT;

    /**
     * @brief Timer for waiting.
     */
    uint32_t timer = 0;

    /**
     * @brief Counter for attempting tasks.
     */
    uint32_t attempts = 0;

    // ---------------------------------------- WIFI PROPS -----------------------------------------

    /**
     * @brief The WiFi client.
     */
    WiFiClient* wifiClient;

    /**
     * @brief Pointer to socket (private) of wifiClient.
     */
    uint8_t* _sock;

    // ---------------------------------------- MQTT PROPS -----------------------------------------

    /**
     * @brief A map of mqtt subscriptions and their callbacks.
     */
    std::map<String, MQTTSubscribe*> mqttSubs;

    /**
     * @brief Birth topic.
     */
    std::pair<String, String> birth_msg;

    // ------------------------------------------ CONNECT ------------------------------------------

    bool connectServer() override;
    bool disconnectServer() override;
    int8_t connect();

    // ----------------------------------------- MESSAGING -----------------------------------------

    /**
     * @brief Handle MQTT messages received.
     */
    void mqttMessageReceived(String &topic, String &payload);

    // ---------------------------- PACKETS, from Adafruit_MQTT_Client -----------------------------

    uint16_t readPacket(uint8_t *buffer, uint16_t maxlen, int16_t timeout) override;
    bool sendPacket(uint8_t *buffer, uint16_t len) override;

    // -------------------------- CONNECTION LOOP - CLOSE SOCKET, RESTART --------------------------

    /**
     * @brief Close connection.
     *
     * @param wifi_connected
     * @return socket is closing
     */
    bool closeConnection(bool wifi_connected);

    /**
     * @brief Finish closing socket.
     * 
     * @param wifi_connected 
     * @return socket closed
     */
    bool closeSocket(bool wifi_connected);

    // ---------------------------------- CONNECTION LOOP - WIFI -----------------------------------

    /**
     * @brief Set WiFi connection up.
     * 
     * @return success
     */
    bool wifiSetup(void);

    /**
     * @brief Connect to WiFi.
     * 
     * @return success
     */
    bool wifiConnect(void);

    // ---------------------------------- CONNECTION LOOP - MQTT -----------------------------------

    /**
     * @brief Connect to MQTT server.
     * 
     * @return success
     */
    bool mqttConnect(void);

    /**
     * @brief Wait on connection to server.
     *
     * @return success
     */
    bool waitOnConnection(void);

    /**
     * @brief Wait after connection to server.
     *
     * @return finished waiting
     */
    bool waitAfterConnection(void);

    /**
     * @brief Connect to MQTT broker.
     * 
     * @return success
     */
    bool mqttConnectBroker(void);

    /**
     * @brief Connect MQTT subscriptions.
     *
     * @return success
     */
    bool mqttSubscribe(void);

    /**
     * @brief Send MQTT announcement.
     * 
     * @return success
     */
    bool mqttAnnounce(void);
};

/**
 * @brief Helper function defined in Adafruit_MQTT.cpp, but not declared.
 */
static uint8_t *stringprint(uint8_t *p, const char *s, uint16_t maxlen = 0);

#endif
