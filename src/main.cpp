#include <Arduino.h>
#include <SPI.h>

#include <WiFi.h>

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <elapsedMillis.h>
#include <string>

#include <ota.h>

#include "kaaro_utils.cpp"
#include <Preferences.h>

#include <IRremote.h>

/* 
    STATICS
*/
const char* mqtt_server = "192.168.4.1";
const uint16_t WAIT_TIME = 1000;
#define BUF_SIZE 75

unsigned long long current_loop_counter = 0;
uint16_t current_brain_counter = 0;

const unsigned long long brain_beat = 1000;

String ROOT_MQ_ROOT = "malboro/";
String PRODUCT_MQ_SUB = "ir/";
String MESSAGE_MQ_STUB = "message";
String COUNT_MQ_STUB = "count";
String OTA_MQ_SUB = "ota/";
long long lastUpdated = 0;
long long THRESHOLD = 3000;
/* 
    FUNCTION DEFINATIONS
*/

void mqttCallback(char *topic, byte *payload, unsigned int length);
void pushEveryLoop();

/* 
 REALTIME VARIABLES
*/

String host = "ytkarta.s3.ap-south-1.amazonaws.com"; // Host => bucket-name.s3.region.amazonaws.com
int port = 80;                                       // Non https. For HTTPS 443. As of today, HTTPS doesn't work.
String bin = "/Malboro/ir-recvr/firmware.bin";       // bin file name with a slash in front.

char mo[75];
String msg = "";
String DEVICE_MAC_ADDRESS;
char ssid[] = "wendor";
char pass[] = "passwordisone";

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

WiFiClient wifiClient;
PubSubClient mqttClient(mqtt_server, 1884, mqttCallback, wifiClient);

elapsedMillis timeElapsed;
Preferences preferences;
#define convertToString(x) #x

int RECV_PIN = 5;
IRrecv irrecv(RECV_PIN);
decode_results results;
//  int RECV_PIN_2 = 23;
// IRrecv irrecv1(RECV_PIN_2);


static int taskCore = 1;
const int mb_pin = 16;

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
 
void coreTask( void * pvParameters ){

 
    String taskMessage = "Task running on core ";
    taskMessage = taskMessage + xPortGetCoreID();
    Serial.println(taskMessage);

    while(true){
        // Serial.println(taskMessage);
        // delay(1000);
    for(int i=0;i<7;i++) {
    onEdge();
            // Serial.print(".");

  }
  for(int i=0;i<10;i++) {
    offEdge();
            // Serial.print("-");

  }

    }
 
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

  String rootTopic = ROOT_MQ_ROOT;
  String readyTopic = ROOT_MQ_ROOT + DEVICE_MAC_ADDRESS;

  String otaTopic = ROOT_MQ_ROOT + OTA_MQ_SUB + DEVICE_MAC_ADDRESS;

  String productMessageTopic = ROOT_MQ_ROOT + PRODUCT_MQ_SUB + MESSAGE_MQ_STUB;
  String productCountTopic = ROOT_MQ_ROOT + PRODUCT_MQ_SUB + COUNT_MQ_STUB;

  String messageTopic = ROOT_MQ_ROOT + MESSAGE_MQ_STUB + '/' + DEVICE_MAC_ADDRESS;
  // String countTopic = ROOT_MQ_ROOT + COUNT_MQ_STUB + DEVICE_MAC_ADDRESS;

  if (topics == otaTopic && msg == "ota")
  {
    Serial.println("Ota Initiating.........");

    OTA_ESP32::execOTA(host, port, bin, &wifiClient);
  }

  else if (topics == "malboro/ota/version")
  {
  }

  if (topics == rootTopic)
  {
    //;;;
  }
  if (topics == productCountTopic)
  {
    Serial.println(msg + " | From count topic");

    preferences.putUInt("target_counter", 0);
  }

  if (topics == productMessageTopic || topics == messageTopic)
  {
    Serial.println(msg + " | From message topic");
    //;;;;
  }
  if (topics == "activate/now")
  {
    Serial.println("Enabling IRin");
    irrecv.enableIRIn(); // Start the receiver
    // irrecv1.enableIRIn();
    Serial.println("Enabled IRin");
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

      String rootTopic = ROOT_MQ_ROOT;                        
      String readyTopic = ROOT_MQ_ROOT + DEVICE_MAC_ADDRESS;

      String otaTopic = ROOT_MQ_ROOT + OTA_MQ_SUB + DEVICE_MAC_ADDRESS;

      String productMessageTopic = ROOT_MQ_ROOT + PRODUCT_MQ_SUB + MESSAGE_MQ_STUB;
      String productCountTopic = ROOT_MQ_ROOT + PRODUCT_MQ_SUB + COUNT_MQ_STUB;

      String messageTopic = ROOT_MQ_ROOT + MESSAGE_MQ_STUB + DEVICE_MAC_ADDRESS;
      String countTopic = ROOT_MQ_ROOT + COUNT_MQ_STUB + DEVICE_MAC_ADDRESS;

      String readyMessage = DEVICE_MAC_ADDRESS + " is Ready.";
      mqttClient.publish(readyTopic.c_str(), "Ready!");
      mqttClient.publish(rootTopic.c_str(), readyMessage.c_str());

      mqttClient.subscribe(rootTopic.c_str());
      mqttClient.subscribe(otaTopic.c_str());

      mqttClient.subscribe(productMessageTopic.c_str());
      mqttClient.subscribe(productCountTopic.c_str());

      mqttClient.subscribe(messageTopic.c_str());
      mqttClient.subscribe(countTopic.c_str());
      mqttClient.subscribe("activate/now");
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

void setup()
{

  Serial.begin(115200);
     pinMode(mb_pin, OUTPUT);
  xTaskCreatePinnedToCore(
                    coreTask,   /* Function to implement the task */
                    "coreTask", /* Name of the task */
                    10000,      /* Stack size in words */
                    NULL,       /* Task input parameter */
                    1,          /* Priority of the task */
                    NULL,       /* Task handle. */
                    taskCore);  /* Core where the task should run */
 
  Serial.println("Task created...");

  DEVICE_MAC_ADDRESS = KaaroUtils::getMacAddress();
  Serial.println(DEVICE_MAC_ADDRESS);
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[0], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.println(mac[5], HEX);
  preferences.begin("malboro", false);
  
  
  char str[100];
  sprintf(str, "%d", target_counter);
  String s = str;

  Serial.print("Connecting Wifi: ");
  
  WiFi.begin(ssid,pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    IPAddress ip = WiFi.localIP();
    Serial.println(ip);
  }
  mqttClient.setCallback(mqttCallback);

  Serial.println("Enabling IRin");
  irrecv.enableIRIn(); // Start the receiver
  // irrecv1.enableIRIn();
  Serial.println("Enabled IRin");

  
}

void loop()
{

  if (WiFi.status() == WL_CONNECTED)
  {

    if (!mqttClient.connected())
    {
      reconnect();
    }
  }
  mqttClient.loop();

    if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    // Serial.println("Picked Item ");
    long long currentMillis = millis();
      if(currentMillis - lastUpdated > THRESHOLD) {
        lastUpdated = millis();
        mqttClient.publish("prod/activate/now","Now");
      }
    irrecv.resume(); // Receive the next value
    // irrecv1.resume();
  }
  delay(10);
  pushEveryLoop();
}


void pushEveryLoop() {
  
  
  if(current_loop_counter%brain_beat == 0) {
    
    Serial.printf("Current loop #%d x %llu \n",current_brain_counter, brain_beat);
    current_brain_counter++;
  }
  current_loop_counter++;
}