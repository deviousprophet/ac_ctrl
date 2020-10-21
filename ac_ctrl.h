#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#include <Arduino.h>
#include <assert.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>

#include "supported_protocols.cpp"

#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "LATITUDE-E7470";
const char* password = "31121998";
const char* mqtt_server = "192.168.137.1";
const int mqtt_port = 1883;

const char* sub_topic = "esp32/admin/test";
const char* pub_topic = "esp32/ac";

WiFiClient espClient;
PubSubClient client(espClient);

const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;
const uint16_t kMinUnknownSize = 12;

bool ac_configed = false;
String configed_protocol;

#if DECODE_AC
const uint8_t kTimeout = 50;
#else
const uint8_t kTimeout = 15;
#endif

void TaskWifi(void *pvParamaters);
void TaskIRsend(void *pvParamaters);