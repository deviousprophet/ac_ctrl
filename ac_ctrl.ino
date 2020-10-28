#include "ac_ctrl.h"

void setup() {
  Serial.begin(115200, SERIAL_8N1);
  while(!Serial) delay(50);
  assert(irutils::lowLevelSanityCheck() == 0); 
#if DECODE_HASH
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif
  irrecv.enableIRIn();
  
  delay(10);
  WiFi.begin(ssid, password);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  xTaskCreate(WifiTask, "wifi_reconnect", 2048, NULL, 3, NULL);
}

void loop() {
  if (irrecv.decode(&results) && !ac_configed) {
    if (results.overflow) Serial.printf(D_WARN_BUFFERFULL "\n", 1024);
    Serial.println(configed_protocol = resultGetProtocol(&results));
    if (configed_protocol != "UNKNOWN") {
      ac_configed = true;
    }
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();
  } else if (ac_configed && ir_send) {
    ir_send = false;
  }
  client.loop();
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String _mess = "";
  String _topic = (String) topic;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    _mess += (char)message[i];
  }
  Serial.println("");

  if ((_topic == AC_TOPIC) && (_mess == "AC_CONFIG"))
    ac_configed = false;
  if ((_topic == AC_POWER) && (_mess == "on") || (_mess == "off"))
    ac_power = _mess;
  if ((_topic == AC_TEMP) && (_mess.toInt() >= 18) && (_mess.toInt() <= 32))
    ac_temp = _mess.toInt();
}

void WifiTask (void *pvParameters) {
  (void) pvParameters;

  for(;;) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Attempting Wifi connection");
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        Serial.print(".");
        vTaskDelay(1000);
      }
      Serial.println("");
      Serial.println("WiFi connected");
    }
    if (!client.connected() && (WiFi.status() == WL_CONNECTED)) {
      Serial.println("Attempting MQTT connection...");
      if (client.connect("ESP_ac_ctrl")) {
        Serial.println("MQTT connected");
        client.subscribe(sub_topic);
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println("");
      }
    }
    vTaskDelay(10);
  }
}
