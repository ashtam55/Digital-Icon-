#include <Arduino.h>
#include <SPI.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <elapsedMillis.h>
#include <string>

#include <Preferences.h>
#include "kaaro_utils.cpp"
/* 
    STATICS
*/
const char *mqtt_server = "api.akriya.co.in";
const uint16_t WAIT_TIME = 1000;
#define BUF_SIZE 75

String ROOT_MQ_ROOT = "digitalicon/";
String PRODUCT_MQ_SUB = "91springboards1/";
String MESSAGE_MQ_STUB = "message";
String COUNT_MQ_STUB = "count";
String OTA_MQ_SUB = "ota/";

String PRODUCT_UNIQUE = " Cowork.Network.Grow ";
int contentLength = 0;
bool isValidContentType = false;

String host = "ytkarta.s3.ap-south-1.amazonaws.com"; // Host => bucket-name.s3.region.amazonaws.com
int port = 80;                                       // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
String bin = "/kaaroMerch/SubsCount/firmware.bin";   // bin file name with a slash in front.

char mo[75];
String msg = "";
String DEVICE_MAC_ADDRESS;
String ssid = "";
String pass = "";

byte mac[6];

int cases = 1;

int fxMode;

uint32_t target_counter = 0;

unsigned long delayStart = 0; // the time the delay started
bool delayRunning = false;
unsigned int interval = 30000;

/*
  HY Variable/Instance creation
*/

elapsedMillis timeElapsed;
Preferences preferences;
#define convertToString(x) #x

const int mb_pin = 16;

const int mb_freq = 38000;
const int mb_ledChannel = 0;
const int mb_resolution = 8;

void setup()
{
  pinMode(mb_pin, OUTPUT);
  // ledcSetup(mb_ledChannel, mb_freq, mb_resolution);
  // ledcSetup(,,)
  // ledcAttachPin(mb_pin, mb_ledChannel);
}


void onEdge() {
  digitalWrite(mb_pin, HIGH);
  delayMicroseconds(13);
  digitalWrite(mb_pin, LOW);
  delayMicroseconds(13);
}

void offEdge() {
  digitalWrite(mb_pin, LOW);
  delayMicroseconds(26);
}
void loop()
{
  for(int i=0;i<7;i++) {
    onEdge();
  }
  for(int i=0;i<10;i++) {
    offEdge();
  }
}
