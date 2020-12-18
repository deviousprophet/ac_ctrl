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

  pinMode(SEND_PIN, OUTPUT);
  digitalWrite(SEND_PIN, LOW);

  xTaskCreate(
    WifiTask,
    "wifi_reconnect",
    2048,
    NULL,
    2,
    NULL);
}

void loop() {
  if (irrecv.decode(&results) && !ac_configed) {
    if (results.overflow) Serial.printf(D_WARN_BUFFERFULL "\n", 1024);
    Serial.println(configed_protocol = resultGetProtocol(&results));
    if (check_supported_protocol(configed_protocol)) ac_configed = true;
    else Serial.println("Unknown or currently not supported!");
    // Display any extra A/C info if we have it.
    String description = IRAcUtils::resultAcToString(&results);
    if (description.length()) Serial.println(D_STR_MESGDESC ": " + description);
    yield();
  }
  if (ac_configed && ir_send) {
    send2ac();
    ir_send = false;
    delay(500);
  }
  client.loop();
}

void callback(char* topic, byte* message, unsigned int length) {
  StaticJsonDocument<256> jsonRecv;
  deserializeJson(jsonRecv, message);
  JsonObject object = jsonRecv.as<JsonObject>();
  bool    power  = object["d"][0]["v"];
  uint8_t temp   = (uint8_t) object["d"][1]["v"];
  uint8_t fan    = fan_cfg(object["d"][2]["v"]);
  
  if !(ac_power == power || ac_temp == temp || ac_fan == fan) {
    ac_power_prev = ac_power;
    ac_power = power;
    ac_temp = temp.toInt();
    ac_fan = fan;

    ir_send = true;
  }

  // Serial.print("Topic: ");
  // Serial.println(topic);
  // Serial.print("Message: ");
  // String _mess2 = "";
  // String _topic = (String) topic;
  
  // for (int i = 0; i < length; i++) {
  //   Serial.print((char)message[i]);
  //   _mess += (char)message[i];
  // }
  // Serial.println("");
  
}

void WifiTask (void *pvParameters) {
  (void) pvParameters;

  for(;;) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect();
      while(WiFi.status() != WL_CONNECTED) vTaskDelay(100);
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

bool check_supported_protocol(String protocol) {
  for (int i = 0; i < 15; i++) {
    if (protocol == supported_protocol[i]) return true;
  }
  return false;
}

void send2ac() {
  // DAIKIN AC
  if (configed_protocol == "DAIKIN") {
    IRDaikinESP ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikinCool);
    ac.setSwingVertical(true);
    ac.setSwingHorizontal(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN2") {
    IRDaikin2 ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikinCool);
    ac.setSwingVertical(kDaikin2SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN216") {
    IRDaikin216 ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikinCool);
    ac.setSwingVertical(true);
    ac.setSwingHorizontal(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN160") {
    IRDaikin160 ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikinCool);
    ac.setSwingVertical(kDaikin160SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "DAIKIN176") {
    IRDaikin176 ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikin176Cool);
    ac.setSwingHorizontal(kDaikin176SwingHAuto);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN128") {
    IRDaikin128 ac(SEND_PIN);
    ac.setPowerToggle(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikin128Cool);
    ac.setSwingVertical(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN152") {
    IRDaikin152 ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikinCool);
    ac.setSwingV(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN64") {
    IRDaikin64 ac(SEND_PIN);
    ac.setPowerToggle(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikin64Cool);
    ac.setSwingVertical(true);
    Serial.println(ac.toString());
    ac.send();
  }
// END DAIKIN
//
// LG

  else if (configed_protocol == "LG") {
    IRLgAc ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kLgAcCool);
    Serial.println(ac.toString());
    ac.send();
  }

// END LG
//
// MITSUBISHI
  else if (configed_protocol == "MITSUBISHI_AC") {
    IRMitsubishiAC ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kMitsubishiAcCool);
    ac.setVane(kMitsubishiAcVaneAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI136") {
    IRMitsubishi136 ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kMitsubishi136Cool);
    ac.setSwingV(kMitsubishi136SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI112") {
    IRMitsubishi112 ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kMitsubishi112Cool);
    ac.setSwingV(kMitsubishi112SwingVAuto);
    ac.setSwingH(kMitsubishi112SwingHAuto);
    Serial.println(ac.toString());
    ac.send();
  }

// END MITSUBISHI
//
// MITSUBISHI HEAVY

  else if (configed_protocol == "MITSUBISHI_HEAVY_152") {
    IRMitsubishiHeavy152Ac ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kMitsubishiHeavyCool);
    ac.setSwingVertical(kMitsubishiHeavy152SwingVAuto);
    ac.setSwingHorizontal(kMitsubishiHeavy152SwingHAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI_HEAVY_88") {
    IRMitsubishiHeavy88Ac ac(SEND_PIN);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kMitsubishiHeavyCool);
    ac.setSwingVertical(kMitsubishiHeavy88SwingVAuto);
    ac.setSwingHorizontal(kMitsubishiHeavy88SwingHAuto);
    Serial.println(ac.toString());
    ac.send();
  }

// END MITSUBISHI HEAVY
//
// SHARP

  else if (configed_protocol == "SHARP_AC") {
    IRSharpAc ac(SEND_PIN);
    ac.setPower(ac_power, ac_power_prev);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kSharpAcCool);
    ac.setSwingToggle(true);
    Serial.println(ac.toString());
    ac.send();
  }
}

uint8_t fan_cfg(String fan) {
  int intFan = fan.toInt();
  
  // DAIKIN, DAIKIN2, DAIKIN152, DAIKIN160, DAIKIN216
  if ((configed_protocol == "DAIKIN") || (configed_protocol == "DAIKIN2") || (configed_protocol == "DAIKIN152") || (configed_protocol == "DAIKIN160") || (configed_protocol == "DAIKIN216")) {
    if ((intFan >= 1) && (intFan <= 5)) return intFan;
    else if (fan = "quiet") return kDaikinFanQuiet;
    else return kDaikinFanAuto;
  
  // DAIKIN176
  if (configed_protocol = "DAIKIN176") {
    if ((intFan >= 1) && (intFan <= 3)) return intFan;
    else return 3;  // FanMax
  }
  // DAIKIN128
  if (configed_protocol = "DAIKIN128") {
    if (fan = "powerful") return kDaikin128FanPowerful;
    else if (fan = "quiet") return kDaikin128FanQuiet;
    else switch (intFan) {
          case 1:
            return kDaikin128FanLow;
            break;
          case 2:
            return kDaikin128FanMed;
            break;
          case 3:
            return kDaikin128FanHigh;
            break;
          default:
            return kDaikin128FanAuto;
            break;
          }
  }
  // DAIKIN64
  if (configed_protocol = "DAIKIN64") {
    if (fan = "quiet") return kDaikin64FanQuiet;
    else if (fan = "turbo") return kDaikin64FanTurbo;
    else switch (intFan) {
          case 1:
            return kDaikin64FanLow;
            break;
          case 2:
            return kDaikin64FanMed;
            break;
          case 3:
            return kDaikin64FanHigh;
            break;
          default:
            return kDaikin64FanAuto;
            break;
          }
  }
  // LG
  if (configed_protocol = "LG") {
    switch (intFan) {
      case 1:
        return kLgAcFanLowest;
        break;
      case 2:
        return kLgAcFanLow;
        break;
      case 3:
        return kLgAcFanMedium;
        break;
      case 4:
        return kLgAcFanHigh;
        break;
      default:
        return kLgAcFanAuto;
        break;
    }
  }
  // MITSUBISHI_AC
  if (configed_protocol = "MITSUBISHI_AC") {
    if ((intFan >= 1) && (intFan <= 5)) return intFan;
    else if (fan = "quiet") return 6;
    else return 0;  // Auto
  }
  // MITSUBISHI136
  if (configed_protocol = "MITSUBISHI136") {
    if ((intFan >= 0) && (intFan <= 3)) return intFan;
    else return 1;     
  }
  // MITSUBISHI112
  if (configed_protocol = "MITSUBISHI112") {
    switch (intFan) {
      case 1:
        return kMitsubishi112FanMin;
        break;
      case 2:
        return kMitsubishi112FanLow;
        break;
      case 3:
        return kMitsubishi112FanMed;
        break;
      case 4:
        return kMitsubishi112FanMax;
        break;
      default:
        return kMitsubishi112FanLow;
        break;
      }
  }
  // MITSUBISHI_HEAVY_152
  if (configed_protocol = "MITSUBISHI_HEAVY_152") {
    if ((intFan >= 1) && (intFan <= 4)) return intFan;
    else if (fan = "econo") return 6;
    else if (fan = "turbo") return 8;
    else return 0;  // Auto
  }
  // MITSUBISHI_HEAVY_88
  if (configed_protocol = "MITSUBISHI_HEAVY_88") {
    if ((intFan >= 2) && (intFan <= 4)) return intFan;
    else if (fan = "econo") return 7;
    else if (fan = "turbo") return 6;
    else return 0;  // Auto
  }
  // SHARP
  if (configed_protocol = "SHARP") {
    switch (intFan) {
      case 1:
        return kSharpAcFanMin;
        break;
      case 2:
        return kSharpAcFanMed;
        break;
      case 3:
        return kSharpAcFanHigh;
        break;
      case 4:
        return kSharpAcFanMax;
        break;
      default:
        return kSharpAcFanAuto;
        break;
      }
  }
}
