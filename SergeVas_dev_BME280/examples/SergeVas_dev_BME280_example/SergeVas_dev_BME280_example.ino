#include <ESP8266WiFi.h>
#include <BME280.h>

const int SDA_PIN = 4; // D2 of WeMos D1 Mini
const int SCL_PIN = 5; // D1 of WeMos D1 Mini

BME280 rbfmiotBme280;

char macAddr[12];

void setup()
{
  int8_t id;

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
}

void loop()
{
  double temp, pres, hum;
  rbfmiotBme280.readAll(&temp, &pres, &hum);
  Serial.print("temp=");
  Serial.print(temp);
  Serial.print("\tpres=");
  Serial.print(pres);
  Serial.print("\thum=");
  Serial.print(hum);
  Serial.println();
  delay(2000);
}

void readMACaddr(char *macAddr)
{
  int i;
  uint8_t macBin[6];
  WiFi.macAddress(macBin);
  for (i = 0; i < sizeof(macBin); i++)
  {
    sprintf(macAddr, "%s%02x", macAddr, macBin[i]);
  }
}
