#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

#include <WiFi.h>
#include <PubSubClient.h>

#define ROOM        "101"
#define AC_TOPIC    "hotel/" ROOM "/admin/ac"
#define AC_POWER    "hotel/" ROOM "/admin/ac/power"
#define AC_TEMP     "hotel/" ROOM "/admin/ac/temp"

const uint16_t RecvPin = 12;
const uint16_t SendPin = 14;

String  configed_protocol;
String  ac_power;
int     ac_temp;
bool    ac_configed = false;
bool    ir_send = false;

const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;
const uint16_t kMinUnknownSize = 12;
#if DECODE_AC
const uint8_t kTimeout = 50;
#else
const uint8_t kTimeout = 15;
#endif

IRrecv irrecv(RecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

const char* ssid = "LATITUDE-E7470";
const char* password = "31121998";
const char* mqtt_server = "192.168.137.1";
const int   mqtt_port = 1883;

const char* sub_topic = "hotel/101/admin/ac/#";
const char* pub_topic = "hotel/101/ac";

WiFiClient espClient;
PubSubClient client(espClient);
