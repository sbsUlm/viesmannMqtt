#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "logger.h"
#include "wifiCredentials.h"
#include "viessmann300.h"

extern "C" {
#include "user_interface.h"
}


//#define mqtt_server "sbspi"
#define mqtt_server "zyklon"
#define ROOT_TOPIC heizung
#define STATUS_TOPIC "ROOT_TOPIC/Status"
#define HEARTBEAT_TOPIC "ROOT_TOPIC/Heartbeat"
#define SET_LOGLEVEL_TOPIC "SetLogLevel"
#define SSID_TOPIC "/WIFI/SSID"
#define IP_TOPIC "/WIFI/IP"
#define CMD_TOPIC "ROOT_TOPIC/Command"
#define SERIAL_TOPIC "ROOT_TOPIC/Serial"
#define BUFFER_SIZE 512               //Buffer size
#define HEARTBEAT 10000               //seconds between keepalive msgs

char sMqttBuffer[BUFFER_SIZE];        //Buffer for MQTT msgs
char sSerialBuffer[BUFFER_SIZE];      //Buffer for serial interface
char sSerialOutBuffer[BUFFER_SIZE];      //Buffer for serial interface
int sInputStringPos = 0;              //Read position in serial buffer
WiFiClient sEspClient;                //WiFi Client
PubSubClient sMqttClient(sEspClient); //MQTT Client
int sMode = -1;                       //the viessmann mode
const Viessmann::Datapoint* sDataPoint;     //the Datapoint ptr

void onMqttData(char* topic, byte* payload, unsigned int length) {
  LOG_INPUT("Got MQTT Topic %s: %s",topic,payload);
  memset(sMqttBuffer,0,BUFFER_SIZE);
  std::string aString(topic);
	if (aString.find(SET_LOGLEVEL_TOPIC) != std::string::npos)
  {
    short aLevel =  ((payload[1] << 8) | payload[0]);
    LOG_INPUT("Setting the loglevel to %d",aLevel);
    Logger::setLogLevel(aLevel);
  }
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void reconnect() {
  // Loop until we're reconnected
  while (!sMqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (sMqttClient.connect("ESP8266Client")) {
    //if (sMqttClient.connect("ESP8266Client")) {
    if (sMqttClient.connect("ESP8266Client","testuser","testpw")) {
      Serial.println("connected");
      sMqttClient.subscribe(CMD_TOPIC);
      snprintf(sMqttBuffer,BUFFER_SIZE,"%s%s",STATUS_TOPIC,SSID_TOPIC);
      sMqttClient.publish(sMqttBuffer,wifi_ssid,true);
      snprintf(sMqttBuffer,BUFFER_SIZE,"%s%s",STATUS_TOPIC,IP_TOPIC);
      sMqttClient.publish(sMqttBuffer,WiFi.localIP().toString().c_str(),true);
      Logger::setClient(&sMqttClient);
      LOG_INFO("Logging enabled");
    } else {
      Serial.print("failed, rc=");
      Serial.print(sMqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void requestTemp()
{
  sDataPoint = Viessmann::Datapoint::getDatapoint(0x5525);
  if (sDataPoint==0)
  {
    LOG_ERROR("invalid Datapoint");
    return;
  }
  LOG_DEBUG("Got Datapoiont Name: %s",sDataPoint->getName().c_str());
  LOG_DEBUG("Got Datapoiont Address: %d",sDataPoint->getAddress());
  int aLen=sDataPoint->createReadRequest(sSerialOutBuffer);
  LOG_DEBUG("Rxd a Request with %d bytes",aLen);
  Serial.write(sSerialOutBuffer,aLen);
}


long lastMsg = 0;
long heartbeatCounter = 0;
void heartbeat()
{

  long now = millis();
  if (now - lastMsg > HEARTBEAT) {
    LOG_DEBUG("sending Hearbeat");
    lastMsg = now;
    snprintf(sMqttBuffer,BUFFER_SIZE,"Uptime %d",(++heartbeatCounter*(HEARTBEAT/1000)));

    sMqttClient.publish(HEARTBEAT_TOPIC, sMqttBuffer, false);
    long aLong=system_get_free_heap_size();
    snprintf(sMqttBuffer,BUFFER_SIZE,"Heapsize %d",aLong );
    sMqttClient.publish(HEARTBEAT_TOPIC, sMqttBuffer, false);
    snprintf(sMqttBuffer,BUFFER_SIZE,"StringPos %d",sInputStringPos );
    sMqttClient.publish(HEARTBEAT_TOPIC, sMqttBuffer, false);

    LOG_DEBUG("requesting temp");
    requestTemp();
    }
}


int switchToMode5()
{
  Serial.print(Viessmann::INIT);
  int aRet=-1;
  int aTries=0;
  while( aRet!=Viessmann::HERATBEAT)
  {
   LOG_DEBUG("Switchting to mode 6");
   size_t len = Serial.available();
   if (len>0)
   {
     if (len>BUFFER_SIZE)
      len=BUFFER_SIZE;
     int aBytesRead = Serial.readBytes(&sSerialBuffer[sInputStringPos], len);
     LOG_INPUT("Rxd %d bytes",aBytesRead);
     if (aBytesRead)
     {
       aRet = sSerialBuffer[aBytesRead-1];
       LOG_INPUT("Last byte is 0x%x",aRet);
       if (aRet ==Viessmann::HERATBEAT)
        return aRet;
      }
   }
    if (++aTries>=3)
      return aRet;
    //wait a second;
    delay(1000);
  }
}


void serialRead()
{
  size_t len = Serial.available();
  if(len){
    Serial.print("Got bytes:");
    if (len>sizeof(sSerialBuffer)) len=sizeof(sSerialBuffer);
    sInputStringPos+= Serial.readBytes(&sSerialBuffer[sInputStringPos], len);
  }
  if (sInputStringPos > 10) {
    Serial.println("Rxd Data");
    sSerialBuffer[sInputStringPos+1] = '\0';
    Serial.println(sSerialBuffer);
    LOG_INPUT(sSerialBuffer);
    memset(sSerialBuffer, 0, BUFFER_SIZE);
    sInputStringPos = 0;
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  sMqttClient.setServer(mqtt_server, 1883);
  sMqttClient.setCallback(onMqttData);

  Viessmann::Datapoint* aNewDataPoint = new Viessmann::Datapoint("TemperatureOutside",false,2,-60,100,0x5525);
  aNewDataPoint = new Viessmann::Datapoint("Kesseltemperatur",false,2,0,150,0x0810);
}


void loop() {
  if (!sMqttClient.connected()) {
    reconnect();
  }
  sMqttClient.loop();
  if (sMode!=Viessmann::HERATBEAT)
  {
    sMode=switchToMode5();
  }
  heartbeat();
  serialRead();
}


#else
#ifndef UNIT_TEST
int main (int argc, char* argv[]) {};
#endif
#endif
