#include "ac_ctrl.h"

const uint16_t kRecvPin = 14;

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
  xTaskCreatePinnedToCore(TaskIRrecv, "IR_recv", 1024, NULL, 3, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(TaskIRsend, "IR_send", 1024, NULL, 3, NULL, ARDUINO_RUNNING_CORE);

}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
  }
  
  if (!client.connected() && (WiFi.status() == WL_CONNECTED)) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP_ac_ctrl")) {
      Serial.println("connected");
      client.subscribe(sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("");
      delay(1000);
    }
  }
  client.loop();
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
    config_ac = true;
  }
}

void TaskIRrecv(void *pvParameters) {
  (void) pvParameters;
  
  for (;;) {
    vTaskDelay(10);
    if (config_ac && irrecv.decode(&results)) {
      if (results.overflow) Serial.printf(D_WARN_BUFFERFULL "\n", kCaptureBufferSize);
      // Display the basic output of what we found.
      Serial.print(resultToHumanReadableBasic(&results));
      // Display any extra A/C info if we have it.
      String description = IRAcUtils::resultAcToString(&results);
      if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
      yield();
    }
  }
}

void TaskIRsend(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    vTaskDelay(10);
    if (config_ac) {
      Serial.println("yes");
      config_ac = false;
    }
  }
}
