#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

#include <ir_Daikin.h>
#include <ir_LG.h>
#include <ir_Mitsubishi.h>
#include <ir_MitsubishiHeavy.h>
#include <ir_Sharp.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define ANALOG_CURRENT_PIN  32
#define RECV_PIN            12
#define SEND_PIN            14

#define ROOM        "101"

String supported_protocol[15] = (
    "DAIKIN",
    "DAIKIN2",
    "DAIKIN160",
    "DAIKIN216",
    "DAIKIN152",
    "DAIKIN176",
    "DAIKIN128",
    "DAIKIN64",
    "LG",
    "MITSUBISHI_AC",
    "MITSUBISHI136",
    "MITSUBISHI112",
    "MITSUBISHI_HEAVY_152",
    "MITSUBISHI_HEAVY_88",
    "SHARP");

String  configed_protocol;
bool    ac_configed = false;
bool    ir_send = false;

bool    ac_power = false;
bool    ac_power_prev = false;
uint8_t ac_temp = 25;
uint8_t ac_fan;

const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;
const uint16_t kMinUnknownSize = 12;
#if DECODE_AC
const uint8_t kTimeout = 50;
#else
const uint8_t kTimeout = 15;
#endif

IRrecv irrecv(RECV_PIN, kCaptureBufferSize, kTimeout, true);
decode_results results;

const char* ssid = "LATITUDE-E7470";
const char* password = "31121998";
const char* mqtt_server = "192.168.137.1";
const int   mqtt_port = 1883;

const char* sub_topic = "json/request";
const char* pub_topic = "json/respond";

WiFiClient espClient;
PubSubClient client(espClient);
