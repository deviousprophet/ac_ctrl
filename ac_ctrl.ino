#include "ac_ctrl.h"

const uint16_t kRecvPin = 12;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup() {
  Serial.begin(kBaudRate);
  while(!Serial) delay(50);
  assert(irutils::lowLevelSanityCheck() == 0);
  Serial.printf("\n" D_STR_IRRECVDUMP_STARTUP "\n", kRecvPin);
  
#if DECODE_HASH
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif
  irrecv.enableIRIn();
  
  delay(10);
  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);  
  xTaskCreatePinnedToCore(TaskWifi, "wifi_reconnect", 1024, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(TaskIRsend, "IR_send", 1024, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
}

void loop() {
  if (!client.connected() && (WiFi.status() == WL_CONNECTED)) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP_ac_ctrl")) {
      Serial.println("connected");
      client.subscribe(sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("");
    }
  }
  client.loop();

  if (irrecv.decode(&results)) {
    if (results.overflow) Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
    configed_protocol = resultGetProtocol(&results);
    if (configed_protocol != "UNKNOWN") ac_configed = true;
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String messageTemp = "";
  String messageTopic = (String) topic;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println("");

  if ((messageTopic == "esp32/admin/test") && (messageTemp == "CONFIG")) {
    ac_configed = false;
  }
}

void TaskWifi(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      Serial.println("Connecting to Wifi...");
      while (WiFi.status() != WL_CONNECTED);
      Serial.println("");
      Serial.println("WiFi connected");
    }
    vTaskDelay(10);
  }
}

void TaskIRsend(void *pvParameters) {
  (void) pvParameters;
  
  for (;;) {
    if (ac_configed) {
      Serial.println(configed_protocol);
      ac_configed = false;
    };
    vTaskDelay(10);
  }
}