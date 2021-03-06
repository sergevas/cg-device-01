#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoNATS.h>
#include <SergeVas_dev_BME280.h>

const int SDA_PIN = 4;  // D2 of WeMos D1 Mini
const int SCL_PIN = 5; // D1 of WeMos D1 Mini

const char* WIFI_SSID = "IoT";
const char* WIFI_PASSWORD = "********";
const int WIFI_NUM_OF_RETRIES = 20;
const uint32_t NETWORK_ERROR_RECOVERY_DELAY = 600000000;

char* NATS_SERVER_NAME="192.168.1.70";
int NATS_SERVER_PORT = 4222;
char* NATS_USER_NAME = "foo";
char* NATS_PASSWORD = "secret";

char* CG_STATUS_SUBJECT = "cg/status";

BME280 rbfmiotBme280(I2C_ADDR_76);
int8_t id;

char macAddr[12];
WiFiClient client;
NATS nats(
  &client,
  "192.168.1.70",
  NATS_DEFAULT_PORT
  );

String dToS(double aVal) {
  return String(aVal, 2);
}

String iToS(int8_t aVal) {
  return String(aVal, 2);
}

char* sToC(String strVal) {
  int strValLength = strVal.length() + 1;
  char *strValArr = (char*)malloc(strValLength);
  strVal.toCharArray(strValArr, strValLength);
  return strValArr;
}

String readMACaddr() {
  int i;
  uint8_t macBin[6];
  String macStr;
  String byt;
  WiFi.macAddress(macBin);
  for (i = 0; i < 6; i++) {
    byt = String(macBin[i], HEX);
    macStr = byt.length() == 1 ? macStr + "0" + byt : macStr + byt;
  }
  return macStr;
}

void initWiFi() {
  Serial.println();
  Serial.println("WiFi connect start...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retriesCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    retriesCount++;
    if (retriesCount > WIFI_NUM_OF_RETRIES) {
      Serial.println();
      Serial.print("Max number of retries for WiFi exceeded "); Serial.println(WIFI_NUM_OF_RETRIES);
      Serial.print("Sleeping for "); Serial.println(NETWORK_ERROR_RECOVERY_DELAY);
      ESP.deepSleep(NETWORK_ERROR_RECOVERY_DELAY);
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected. IP address "); Serial.println(WiFi.localIP());
  Serial.println("WiFi connect complete...");
}

void nats_handler(NATS::msg msg) {
  double temp, pres, hum;
  rbfmiotBme280.readAll(&temp, &pres, &hum);
  
  String replyMsg = "{\"id\":" + iToS(id)
    + ", \"temp\":" + dToS(temp)
    + ", \"pres\":" + dToS(pres)
    + ", \"hum\":" + dToS(hum) + "}";
    
  nats.publish(msg.reply, sToC(replyMsg));
}

void nats_on_connect() {
  String macAddr = readMACaddr();
  String subject = "cg/" + macAddr;
  nats.subscribe(sToC(subject), nats_handler);
  nats.publish(CG_STATUS_SUBJECT, sToC(subject));
}



void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Configure start...");
  rbfmiotBme280.configure(SDA_PIN, SCL_PIN);
  Serial.println("Configure complete...");
  Serial.println("Reading device id start...");
  rbfmiotBme280.readId(&id);
  Serial.print("id 0x");
  Serial.println(id, HEX);
  Serial.println("Reading device id complete...");
  Serial.println(readMACaddr());
  nats.on_connect = nats_on_connect;
  nats.connect(); 
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }
  nats.process();
  yield();
}
