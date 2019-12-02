#include <Arduino.h>
#include <SPI.h>

#include <YoutubeApi.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <elapsedMillis.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#include <ota.h>
#include "display_kaaro.h"
#include "kaaro_utils.cpp"

#include <kaaroTouchAdmin.cpp>

/* 
    STATICS
*/
BLECharacteristic *pCharacteristic;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
BLECharacteristic *pNotifyCharacteristic;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "e3327c31-3123-4a99-bf27-b900c24c4e68"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define API_KEY "AIzaSyBQeMMEWAZNErbkgtcvF6iaJFW4237Vkfw"
#define CHANNEL_ID "UC_vcKmg67vjMP7ciLnSxSHQ"
const char *mqtt_server = "api.akriya.co.in";
const uint16_t WAIT_TIME = 1000;
#define BUF_SIZE 75

/* 
    FUNCTION DEFINATIONS
*/

void displayScroll(char *pText, textPosition_t align, textEffect_t effect, uint16_t speed);
void mqttCallback(char *topic, byte *payload, unsigned int length);

   
/* 
 REALTIME VARIABLES
*/
BLEService *pService = NULL;
BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
String bledata = "Hola!!";
int contentLength = 0;
bool isValidContentType = false;

bool forble = false;
unsigned long api_mtbs = 10000;
unsigned long api_lasttime;

long subs = 0;
int buttonState= 0;
int lastState = 0;

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

uint32_t current_counter = 0;
uint32_t target_counter = 0;

unsigned long delayStart = 0; // the time the delay started
bool delayRunning = false;
unsigned int interval = 12000;

int total_touch_pins = 1;
int pin_numbers[1] = {12};

/*
  HY Variable/Instance creation
*/

WiFiClient wifiClient;
PubSubClient mqttClient(mqtt_server, 1883, mqttCallback, wifiClient);
WiFiManager wifiManager;
WiFiClientSecure client;
YoutubeApi api(API_KEY, client);
DigitalIconDisplay display;
elapsedMillis timeElapsed;

void mqtt(int state){
if (state == 1){
      Serial.println("Next");
      mqttClient.publish("kaaroEvent/dev1/screen1/input", "NEXT");

}
else if(state == 0){
        // mqttClient.publish("kaaroEvent/dev1/screen1/input", "OPEN");

}

}

class MyServerCallbacks : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;

  };

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
  }
};

class MyCallbacks : public BLECharacteristicCallbacks
{

  void onWrite(BLECharacteristic *pCharacteristic)
  {

    std::string rxValue = pCharacteristic->getValue();
    bledata = "";
    if (rxValue.length() > 0)
    {
      Serial.println("*********");
      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++)
      {

        bledata.concat(rxValue[i]);

        Serial.print(rxValue[i]);
      }

      Serial.println();
      Serial.println("*********");
      Serial.print("bledata = ");
      Serial.print(bledata);
      Serial.println();

    }

    if(bledata.startsWith("c/")){
    bledata = bledata.substring(2);
    display.updateCounterValue(bledata,true);

    }
    else if(bledata.startsWith("m/")){
    bledata = bledata.substring(2);
    display.showCustomMessage(bledata);
    }
  }
};

void checkTouchpin(){
  
buttonState = kaaroTouchAdmin::getPinState(1);
// Serial.printf("Button State = %d \n",buttonState);
  // compare the buttonState to its previous state
  if (buttonState != lastState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      // if the current state is HIGH then the button went from off to on:
      forble = true;
      mqtt(1);
      // Serial.println("on");
    } else {
      mqtt(0);
      // if the current state is LOW then the button went from on to off:
      // Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(100);
  }
  // save the current state as the last state, for next time through the loop
  lastState = buttonState;
}

void mqttCallback(char *topic, uint8_t *payload, unsigned int length)
{
  char *cleanPayload = (char *)malloc(length + 1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length + 1);
  msg = String(cleanPayload);
  free(cleanPayload);

  String topics = String(topic);
  Serial.print("From MQTT = ");
  Serial.println(msg);

  if (topics == "kaaroEvent/dev1/ota" && msg == "ota")
  {
    Serial.println("Ota Initiating.........");

    OTA_ESP32::execOTA(host, port, bin, &wifiClient);
  }

  else if (topics == "kaaroEvent/dev1/ota/version")
  {
  }

  if (topics == "digitalicon/")
  {
    display.showCustomMessage(msg);
  }
  if (topics == "kaaroEvent/dev1/count")
  {
    Serial.println("From count topic");
    display.updateCounterValue(msg, true);
  }

    if (topics == "kaaroEvent/dev1/message")
  {
    Serial.println("From message topic");
    display.showCustomMessage(msg);
  }

}


void reconnect()
{

  while (!mqttClient.connected())
  {
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println("connected");

      String readyTopic = "kaaroEvent/dev1/" + DEVICE_MAC_ADDRESS;
      mqttClient.publish(readyTopic.c_str(), "Ready!");
      mqttClient.publish("digitalicon", "Ready!");

      mqttClient.subscribe("kaaroEvent/dev1/ota");
      mqttClient.subscribe("kaaroEvent/dev1/");
      mqttClient.subscribe("kaaroEvent/dev1/message");
      String otaTopic = "kaaroEvent/dev1/" + DEVICE_MAC_ADDRESS;
      mqttClient.subscribe(otaTopic.c_str());

      String msgTopic = "kaaroEvent/dev1/" + DEVICE_MAC_ADDRESS;
      mqttClient.subscribe(msgTopic.c_str());

      mqttClient.subscribe("kaaroEvent/dev1/count/");
      String countTopic = "kaaroEvent/dev1/count/" + DEVICE_MAC_ADDRESS;
      mqttClient.subscribe(countTopic.c_str());
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void initBle()
{
  
  String noBleName = "DI-" + KaaroUtils::getMacAddress();
    BLEDevice::init(noBleName.c_str());
    BLEDevice::setMTU(512);
    
    pServer = BLEDevice::createServer();

    pService = pServer->createService(SERVICE_UUID);

  pNotifyCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID_TX,
      BLECharacteristic::PROPERTY_NOTIFY);
  pNotifyCharacteristic->addDescriptor(new BLE2902());

   pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // pCharacteristic->setValue("Hello World");
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();

  Serial.println("Waiting a client connection to notify...");
}

void setup()
{

  Serial.begin(115200);
  initBle();
  DEVICE_MAC_ADDRESS = KaaroUtils::getMacAddress();
  Serial.println(DEVICE_MAC_ADDRESS);
  WiFi.macAddress(mac);
    Serial.print("MAC: ");
    Serial.print(mac[0],HEX);
    Serial.print(":");
    Serial.print(mac[1],HEX);
    Serial.print(":");
    Serial.print(mac[2],HEX);
    Serial.print(":");
    Serial.print(mac[3],HEX);
    Serial.print(":");
    Serial.print(mac[4],HEX);
    Serial.print(":");
    Serial.println(mac[5],HEX);

  kaaroTouchAdmin::setTouchPinConfig(pin_numbers,total_touch_pins);
  display.setupIcon();


  Serial.print("Connecting Wifi: ");
  wifiManager.setConnectTimeout(5);

  wifiManager.setConfigPortalBlocking(false);
  wifiManager.setWiFiAutoReconnect(true);
  wifiManager.autoConnect("Digital Icon");

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);

    ssid = WiFi.SSID();
    pass = WiFi.psk();

    // display.showCustomMessage(" AQI");
  }

  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);
  display.updateCounterValue("6969", true);

}

void loop()
{
  

  kaaroTouchAdmin::loop();
  wifiManager.process();

  checkTouchpin();


  if (timeElapsed > interval) 
  {

    char a[50];
    String forBLE = "DeviceID : " + DEVICE_MAC_ADDRESS;
    forBLE.toCharArray(a,forBLE.length()+1);
    pNotifyCharacteristic->setValue(a);
      // pCharacteristic->setValue("Send");
    pNotifyCharacteristic->setNotifyProperty(true);
    pNotifyCharacteristic->notify(); 
    delay(10);
      Serial.print("From here");
  //     display.showCustomMessage(" Cowork.Network.Grow ");

  //   // display.showCustomMessage(" Total ");
      
        switch (cases)
        {
        case 1:
        display.bullseye();
        cases = 2;
        break;
        case 2:
        display.bounce();
        cases = 3;
        break;
        case 3:
        display.spiral();
        cases = 4;
        break;
        case 4:
        display.showCustomMessage(" Cowork.Network.Grow ");
        cases = 5;
        break;
        case 5:
        display.transformation1();
        cases = 6;
        break;
        case 6:
        display.showCustomMessage(" #91Life ");
        cases = 1;
        break;
        }
    timeElapsed = 0;
  }
  if (WiFi.status() == WL_CONNECTED)
  {

    if (!mqttClient.connected())
    {
      reconnect();
    }
  }
  
    if (deviceConnected && forble)
  {

    Serial.println("Sending data to Ble");
    char a[100];
    String forBLE = "DeviceID : " + DEVICE_MAC_ADDRESS + "Switch State : HIGH ";
    forBLE.toCharArray(a,forBLE.length()+1);
    pNotifyCharacteristic->setValue("State : 1");
    pNotifyCharacteristic->setNotifyProperty(true);
    pNotifyCharacteristic->notify(); 
    delay(10);
    forble = false;

    // delay(10); // bluetooth stack will go into congestion, if too many packets are sent
}
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500);                  // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
    if (deviceConnected)
  {
    Serial.println("disconnected");
    pServer->disconnect(pServer->getConnId());
  }
  mqttClient.loop(); 
  display.loop();
}

