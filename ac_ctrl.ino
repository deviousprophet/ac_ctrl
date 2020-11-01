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
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  xTaskCreate(WifiTask, "wifi_reconnect", 2048, NULL, 2, NULL);
  xTaskCreate(CurrentMeasure, "current_measure", 2048, NULL, 1, NULL);
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
  }
  if (ac_configed && ir_send) {
    Serial.println("send");
    send2ac();
    Serial.println("send2");
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

  if (_topic == AC_PROTOCOL) configed_protocol = _mess;

  if ((_topic == AC_POWER) && (_mess == "on") || (_mess == "off")) {
    if (_mess == "on") ac_power = true;
    else ac_power = false;
    ir_send = true;
  }

  if ((_topic == AC_TEMP) && (_mess.toInt() >= 18) && (_mess.toInt() <= 32)) {
    ac_temp = (uint8_t) _mess.toInt();
    ir_send = true;
  }
}

void WifiTask (void *pvParameters) {
  (void) pvParameters;

  for(;;) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
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

void CurrentMeasure (void *pvParameter) {
  (void) pvParameter;

  for (;;) {
    int val = analogRead(ANALOG_CURRENT_PIN);
    float adc = (float)val*3.3/4095;
    vTaskDelay(10);
  }
}

void send2ac() {
  // DAIKIN AC
  if (configed_protocol == "DAIKIN") {
    IRDaikinESP ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kDaikinFanAuto);
    ac.setMode(kDaikinAuto);
    ac.setSwingVertical(true);
    ac.setSwingHorizontal(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN2") {
    IRDaikin2 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kDaikinFanAuto);
    ac.setMode(kDaikinAuto);
    ac.setSwingVertical(kDaikin2SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN216") {
    IRDaikin216 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kDaikinFanAuto);
    ac.setMode(kDaikinAuto);
    ac.setSwingVertical(true);
    ac.setSwingHorizontal(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN160") {
    IRDaikin160 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kDaikinFanAuto);
    ac.setMode(kDaikinAuto);
    ac.setSwingVertical(kDaikin160SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "DAIKIN176") {
    IRDaikin176 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setMode(kDaikin176Auto);
    ac.setFan(kDaikin176FanMax);
    ac.setSwingHorizontal(kDaikin176SwingHAuto);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN128") {
    IRDaikin128 ac(SendPin);
    ac.setPowerToggle(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kDaikin128FanAuto);
    ac.setMode(kDaikin128Auto);
    ac.setSwingVertical(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN152") {
    IRDaikin152 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kDaikinFanAuto);
    ac.setMode(kDaikinAuto);
    ac.setSwingV(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN64") {
    IRDaikin64 ac(SendPin);
    ac.setPowerToggle(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kDaikin64FanAuto);
    ac.setMode(kDaikin64Cool);
    ac.setSwingVertical(true);
    Serial.println(ac.toString());
    ac.send();
  }
// END DAIKIN AC
//
// MITSUBISHI
  else if (configed_protocol == "MITSUBISHI_AC") {
    IRMitsubishiAC ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kMitsubishiAcFanAuto);
    ac.setMode(kMitsubishiAcAuto);
    ac.setVane(kMitsubishiAcVaneAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI136") {
    IRMitsubishi136 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kMitsubishi136FanMed);
    ac.setMode(kMitsubishi136Auto);
    ac.setSwingV(kMitsubishi136SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI112") {
    IRMitsubishi112 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kMitsubishi112FanMed);
    ac.setMode(kMitsubishi112Auto);
    ac.setSwingV(kMitsubishi112SwingVAuto);
    ac.setSwingH(kMitsubishi112SwingHAuto);
    Serial.println(ac.toString());
    ac.send();
  }

// END MITSUBISHI
//
// MITSUBISHI HEAVY

  else if (configed_protocol == "MITSUBISHI_HEAVY_152") {
    IRMitsubishiHeavy152Ac ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(kMitsubishiHeavy152FanAuto);
    ac.setMode(kMitsubishiHeavyAuto);
    ac.setSwingVertical(kMitsubishiHeavy152SwingVAuto);
    ac.setSwingHorizontal(kMitsubishiHeavy152SwingHAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI_HEAVY_88") {
    IRMitsubishiHeavy88Ac ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan();
    ac.setMode();
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI112") {
    IRMitsubishi112 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan();
    ac.setMode();
    Serial.println(ac.toString());
    ac.send();
  }
  

// END MITSUBISHI HEAVY
  ir_send = false;
  delay(500);
}
