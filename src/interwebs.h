#ifndef CRYPTID_INTERWEBS_H
#define CRYPTID_INTERWEBS_H

#include <map>
#include <WiFiNINA.h>
#include <utility/wifi_drv.h>
#include <utility/server_drv.h>
#include <utility/WiFiSocketBuffer.h>
#include <MQTT.h>

// Note: These pin definitions leave the the ESP32's `GPIO0` pin undefined (-1). If you wish to use
// this pin - solder the pad on the bottom of the FeatherWing and set `#define ESP32_GPIO0` to the
// correct pin for your microcontroller.
// @see https://learn.adafruit.com/adafruit-airlift-featherwing-esp32-wifi-co-processor-featherwing/arduino
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    13   // Chip select pin
#define ESP32_RESETN  12   // Reset pin
#define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1

#define MQTT_CLIENT_ID "cryptidBottles"
#define MQTT_USER "cryptid"
#define MQTT_PASS "public"

//-------- Rob MQTTClient Private Methods/Properties --------

template<typename Tag>
struct robbed {
  // export it
  typedef typename Tag::type type;
  static type ptr;
};

template<typename Tag>
typename robbed<Tag>::type robbed<Tag>::ptr;

template<typename Tag, typename Tag::type p>
struct rob : robbed<Tag> {
  // fill it
  struct filler {
    filler() { robbed<Tag>::ptr = p; }
  };
  static filler filler_obj;
};

template<typename Tag, typename Tag::type p>
typename rob<Tag, p>::filler rob<Tag, p>::filler_obj;

// &MQTTClient::_connected
struct MQTTClientConnected { typedef bool MQTTClient::*type; };
template class rob<MQTTClientConnected, &MQTTClient::_connected>;

// &MQTTClient::_lastError
struct MQTTClientLastError { typedef lwmqtt_err_t MQTTClient::*type; };
template class rob<MQTTClientLastError, &MQTTClient::_lastError>;

// &MQTTClient::netClient
struct MQTTClientNetClient { typedef Client* MQTTClient::*type; };
template class rob<MQTTClientNetClient, &MQTTClient::netClient>;

// &MQTTClient::client
struct MQTTClientClient { typedef lwmqtt_client_t MQTTClient::*type; };
template class rob<MQTTClientClient, &MQTTClient::client>;

// &MQTTClient::will
struct MQTTClientWill { typedef lwmqtt_will_t* MQTTClient::*type; };
template class rob<MQTTClientWill, &MQTTClient::will>;

// &MQTTClient::returnCode
struct MQTTClientReturnCode { typedef lwmqtt_return_code_t MQTTClient::*type; };
template class rob<MQTTClientReturnCode, &MQTTClient::_returnCode>;

// &WiFiClient::_sock
struct WiFiClientSock { typedef uint8_t WiFiClient::*type; };
template class rob<WiFiClientSock, &WiFiClient::_sock>;

//-------- End MQTTClient Robbery --------

/**
 * @brief Interwebs connection status.
 */
typedef enum {
  INTERWEBS_STATUS_INIT = 0,
  INTERWEBS_STATUS_WIFI_CONNECTING = 1,
  INTERWEBS_STATUS_WIFI_CONNECTED = 2,
  INTERWEBS_STATUS_WIFI_OFFLINE = 3,
  INTERWEBS_STATUS_WIFI_ERRORS = 4,
  INTERWEBS_STATUS_MQTT_CONNECTING = 10,
  INTERWEBS_STATUS_MQTT_CLOSING_SOCKET = 15,
  INTERWEBS_STATUS_MQTT_CONNECTING_2 = 14,
  INTERWEBS_STATUS_MQTT_CONNECTION_SUCCESS = 17,
  INTERWEBS_STATUS_MQTT_CONNECTED = 11,
  INTERWEBS_STATUS_MQTT_OFFLINE = 12,
  INTERWEBS_STATUS_MQTT_SUBSCRIPTION_FAIL = 16,
  INTERWEBS_STATUS_MQTT_ERRORS = 13,
} interwebs_status_t;

/**
 * @brief MQTT subscription callback function. Param is payload.
 */
typedef std::function<void(String&)> mqttcallback_t;

/**
 * @brief Connect to the interwebs and discover all the interesting webs.
 */
class Interwebs {
  public:
    /**
     * @brief Construct a new Interwebs object.
     */
    Interwebs();

    /**
     * @brief Connect to WiFi. Run in setup() after subscription hooks.
     *
     * @return Whether the connection was successful.
     */
    bool connect(void);

    /**
     * @brief Connect to the given WiFi network, using password.
     * 
     * @return bool 
     */
    bool wifiInit(void);

    /**
     * @brief Reconnect to WiFi. This method operates step-by-step, continuing each call.
     * 
     * @return Connected
     */
    bool wifiReconnect(void);

    /**
     * @brief WiFi connection.
     */
    bool wifiIsConnected(void);

    /**
     * @brief Print WiFi status over the Serial connection.
     */
    void printWifiStatus(void);

    /**
     * @brief Initialize MQTT client.
     *
     * @return Success
     */
    bool mqttInit(void);

    /**
     * @brief Reconnect to MQTT broker. This method operates step-by-step, continuing each call.
     * 
     * @return Connected
     */
    bool mqttReconnect(void);

    /**
     * @brief MQTT connection.
     */
    bool mqttIsConnected(void);

    /**
     * @brief Verify connection to WiFi and MQTT.
     * 
     * @return Connected
     */
    bool verifyConnection(void);

    /**
     * @brief Main MQTT client loop. Run on main loop.
     */
    void mqttLoop(void);

    /**
     * @brief MQTT hook.
     */
    void onMqtt(String topic, mqttcallback_t callback);

    /**
     * @brief Send MQTT message. Verifies connection before sending.
     */
    void mqttSendMessage(String topic, String payload);

    /**
     * @brief Send all discoveries.
     *
     * @return Success
     */
    bool mqttSendDiscovery(void);

    /**
     * @brief Connect MQTT subscriptions.
     *
     * @return All subscribed.
     */
    bool mqttSubscribe(void);

  private:
    /**
     * @brief Current status of interwebs connections.
     */
    int status = INTERWEBS_STATUS_INIT;

    /**
     * @brief A map of mqtt subscriptions and their callbacks.
     */
    std::map<String, mqttcallback_t> mqttSubs;

    /**
     * @brief The WiFi client.
     */
    WiFiClient wifiClient;

    /**
     * @brief The MQTT client.
     */
    MQTTClient *mqttClient;

    /**
     * @brief The IP address to connect to.
     */
    IPAddress mqttBroker;

    /**
     * @brief Handle MQTT messages received.
     */
    void mqttMessageReceived(String &topic, String &payload);
};

#endif
