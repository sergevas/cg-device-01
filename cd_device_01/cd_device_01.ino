#include <ArduinoNATS.h>

#include <SergeVas_dev_BME280.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const int SDA_PIN = 4;  // D2 of WeMos D1 Mini
const int SCL_PIN = 5; // D1 of WeMos D1 Mini

const char* WIFI_SSID = "IoT";
const char* WIFI_PASSWORD = "VeryL0ngPas$wd!2015";
const int WIFI_NUM_OF_RETRIES = 20;

ESP8266WebServer server(80);

BME280 rbfmiotBme280(I2C_ADDR_76);
int8_t id;

char macAddr[12];
WiFiClient client;

String dToS(double aVal) {
  return String(aVal, 2);
}

String iToS(int8_t aVal) {
  return String(aVal, 2);
}

void handleRoot() {
  double temp, pres, hum;
  rbfmiotBme280.readAll(&temp, &pres, &hum);
  
  String replyMsg = "{\"id\":" + iToS(id)
    + ", \"temp\":" + dToS(temp)
    + ", \"pres\":" + dToS(pres)
    + ", \"hum\":" + dToS(hum) + "}";
    
  server.send(200, "application/json", "");
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

//char* sToC(String aVal) {
//  int strValLength = strVal.length() + 1;
//  char *strValArr = (char*)malloc(strValLength);
//  strVal.toCharArray(strValArr, strValLength);
//  return strValArr;
//}

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
  readMACaddr(macAddr);
  Serial.println(macAddr);
  initWiFi();
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    initWiFi();
  }
  server.handleClient();
  MDNS.update();
}
