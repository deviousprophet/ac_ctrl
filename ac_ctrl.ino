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

  pinMode(SendPin, OUTPUT);
  digitalWrite(SendPin, LOW);

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
    send2ac();
    ir_send = false;
    delay(500);
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
  
  if ((_topic == AC_TOPIC) && (_mess == "AC_CONFIG")) {
    ac_power = false;
    ac_power_prev = false;
    ac_temp = 25;
    ac_configed = false;
  }

  if (_topic == AC_PROTOCOL) {
    configed_protocol = _mess;
    ac_configed = true;
  }

  if (ac_configed) ac_data(_topic, _mess);
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

void CurrentMeasure (void *pvParameter) {
  (void) pvParameter;
  float maxCurrent = 30;
  for (;;) {
    int val = analogRead(ANALOG_CURRENT_PIN);
    float adc = (float)val*maxCurrent/4095;
    vTaskDelay(100);
  }
}

void send2ac() {
  // DAIKIN AC
  if (configed_protocol == "DAIKIN") {
    IRDaikinESP ac(SendPin);
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
    IRDaikin2 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikinCool);
    ac.setSwingVertical(kDaikin2SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN216") {
    IRDaikin216 ac(SendPin);
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
    IRDaikin160 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikinCool);
    ac.setSwingVertical(kDaikin160SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "DAIKIN176") {
    IRDaikin176 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikin176Cool);
    ac.setSwingHorizontal(kDaikin176SwingHAuto);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN128") {
    IRDaikin128 ac(SendPin);
    ac.setPowerToggle(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikin128Cool);
    ac.setSwingVertical(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN152") {
    IRDaikin152 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kDaikinCool);
    ac.setSwingV(true);
    Serial.println(ac.toString());
    ac.send();
  }
  else if (configed_protocol == "DAIKIN64") {
    IRDaikin64 ac(SendPin);
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
    IRLgAc ac(SendPin);
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
    IRMitsubishiAC ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kMitsubishiAcCool);
    ac.setVane(kMitsubishiAcVaneAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI136") {
    IRMitsubishi136 ac(SendPin);
    ac.setPower(ac_power);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kMitsubishi136Cool);
    ac.setSwingV(kMitsubishi136SwingVAuto);
    Serial.println(ac.toString());
    ac.send();
  }

  else if (configed_protocol == "MITSUBISHI112") {
    IRMitsubishi112 ac(SendPin);
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
    IRMitsubishiHeavy152Ac ac(SendPin);
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
    IRMitsubishiHeavy88Ac ac(SendPin);
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
    IRSharpAc ac(SendPin);
    ac.setPower(ac_power, ac_power_prev);
    ac.setTemp(ac_temp);
    ac.setFan(ac_fan);
    ac.setMode(kSharpAcCool);
    ac.setSwingToggle(true);
    Serial.println(ac.toString());
    ac.send();
  }
}

void ac_data(String _topic, String _mess) {
  int intMess = _mess.toInt();
  if (_topic == AC_POWER) {
    ac_power_prev = ac_power;
    if (_mess == "on") ac_power = true;
    else ac_power = false;
    ir_send = true;
  }

  if (_topic == AC_TEMP) {
    ac_temp = (uint8_t) _mess.toInt();
    ir_send = true;
  }
  if (_topic == AC_FAN) {

    // DAIKIN, DAIKIN2, DAIKIN152, DAIKIN160, DAIKIN216
    if ((configed_protocol == "DAIKIN") || (configed_protocol == "DAIKIN2") || (configed_protocol == "DAIKIN152") || (configed_protocol == "DAIKIN160") || (configed_protocol == "DAIKIN216")) {
      if ((intMess >= 1) && (intMess <= 5)) ac_fan = intMess;
      else if (_mess = "quiet") ac_fan = kDaikinFanQuiet;
      else ac_fan = kDaikinFanAuto;
    }
    // DAIKIN176
    if (configed_protocol = "DAIKIN176") {
      if ((intMess >= 1) && (intMess <= 3)) ac_fan = intMess;
      else ac_fan = 3;  // FanMax
    }
    // DAIKIN128
    if (configed_protocol = "DAIKIN128") {
      if (_mess = "powerful") ac_fan = kDaikin128FanPowerful;
      else if (_mess = "quiet") ac_fan = kDaikin128FanQuiet;
      else switch (intMess) {
            case 1:
              ac_fan = kDaikin128FanLow;
              break;
            case 2:
              ac_fan = kDaikin128FanMed;
              break;
            case 3:
              ac_fan = kDaikin128FanHigh;
              break;
            default:
              ac_fan = kDaikin128FanAuto;
              break;
            }
    }
    // DAIKIN64
    if (configed_protocol = "DAIKIN64") {
      if (_mess = "quiet") ac_fan = kDaikin64FanQuiet;
      else if (_mess = "turbo") ac_fan = kDaikin64FanTurbo;
      else switch (intMess) {
            case 1:
              ac_fan = kDaikin64FanLow;
              break;
            case 2:
              ac_fan = kDaikin64FanMed;
              break;
            case 3:
              ac_fan = kDaikin64FanHigh;
              break;
            default:
              ac_fan = kDaikin64FanAuto;
              break;
            }
    }
    // LG
    if (configed_protocol = "LG") {
      switch (intMess) {
        case 1:
          ac_fan = kLgAcFanLowest;
          break;
        case 2:
          ac_fan = kLgAcFanLow;
          break;
        case 3:
          ac_fan = kLgAcFanMedium;
          break;
        case 4:
          ac_fan = kLgAcFanHigh;
          break;
        default:
          ac_fan = kLgAcFanAuto;
          break;
      }
    }
    // MITSUBISHI_AC
    if (configed_protocol = "MITSUBISHI_AC") {
      if ((intMess >= 1) && (intMess <= 5)) ac_fan = intMess;
      else if (_mess = "quiet") ac_fan = 6;
      else ac_fan = 0;  // Auto
    }
    // MITSUBISHI136
    if ((configed_protocol = "MITSUBISHI136") && (intMess >= 0) && (intMess <= 3)) ac_fan = intMess;
    // MITSUBISHI112
    if (configed_protocol = "MITSUBISHI112") {
      switch (intMess) {
        case 1:
          ac_fan = kMitsubishi112FanMin;
          break;
        case 2:
          ac_fan = kMitsubishi112FanLow;
          break;
        case 3:
          ac_fan = kMitsubishi112FanMed;
          break;
        case 4:
          ac_fan = kMitsubishi112FanMax;
          break;
        default:
          ac_fan = kMitsubishi112FanLow;
          break;
        }
    }
    // MITSUBISHI_HEAVY_152
    if (configed_protocol = "MITSUBISHI_HEAVY_152") {
      if ((intMess >= 1) && (intMess <= 4)) ac_fan = intMess;
      else if (_mess = "econo") ac_fan = 6;
      else if (_mess = "turbo") ac_fan = 8;
      else ac_fan = 0;  // Auto
    }
    // MITSUBISHI_HEAVY_88
    if (configed_protocol = "MITSUBISHI_HEAVY_88") {
      if ((intMess >= 2) && (intMess <= 4)) ac_fan = intMess;
      else if (_mess = "econo") ac_fan = 7;
      else if (_mess = "turbo") ac_fan = 6;
      else ac_fan = 0;  // Auto
    }
    // SHARP
    if (configed_protocol = "SHARP") {
      switch (intMess) {
        case 1:
          ac_fan = kSharpAcFanMin;
          break;
        case 2:
          ac_fan = kSharpAcFanMed;
          break;
        case 3:
          ac_fan = kSharpAcFanHigh;
          break;
        case 4:
          ac_fan = kSharpAcFanMax;
          break;
        default:
          ac_fan = kSharpAcFanAuto;
          break;
        }
    }

    ir_send = true;
  }
}