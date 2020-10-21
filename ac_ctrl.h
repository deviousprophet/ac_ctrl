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

#include <ir_Daikin.h>
#include <ir_Mitsubishi.h>
#include <ir_MitsubishiHeavy.h>
#include <ir_NEC.h>
#include <ir_Sharp.h>

#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Hotspot-LATITUDE-E6420";
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

bool config_ac = false;
bool ac_configed = false;

#if DECODE_AC
const uint8_t kTimeout = 50;
#else
const uint8_t kTimeout = 15;
#endif

void TaskIRrecv(void *pvParamaters);
void TaskIRsend(void *pvParamaters);
