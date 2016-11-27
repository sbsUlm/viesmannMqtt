#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "logger.h"
#include "wifiCredentials.h"
#include "viessmann300.h"
#include <ESP8266mDNS.h>
#include "ArduinoOTA.h"

extern "C" {
#include "user_interface.h"
}


#define mqtt_server "sbspi"
//#define mqtt_server "zyklon"

#define STATUS_TOPIC "heizung/Status"
#define HEARTBEAT_TOPIC "heizung/Heartbeat"
#define DATAPOINT_TOPIC "heizung/DataPoint"
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

void publishDataPoints()
{
  Viessmann::Datapoint::DatapointIteratorT aIt = Viessmann::Datapoint::getDatapointIt();
  char aChar[8];
  for (aIt;aIt!=Viessmann::Datapoint::getEndIt();aIt++)
  {
    snprintf(sMqttBuffer,BUFFER_SIZE,"%s/%s",DATAPOINT_TOPIC,aIt->second->getName().c_str());
    snprintf(aChar,8,"%d",aIt->second->getValueAsShort());
    sMqttClient.publish(sMqttBuffer,aChar,true);
  }
}

void requestTemp()
{
  sDataPoint = Viessmann::Datapoint::getIterator()->second;
  if (sDataPoint==0)
  {
    LOG_ERROR("invalid Datapoint");
    return;
  }
  LOG_INFO("Request Datapoiont Name: %s",sDataPoint->getName().c_str());
  LOG_DEBUG("Request Datapoiont Address: %d",sDataPoint->getAddress());
  int aLen=sDataPoint->createReadRequest(sSerialOutBuffer);
  LOG_DEBUG("Txing a Request with %d bytes",aLen);
  Serial.write(sSerialOutBuffer,aLen);
  for (int ii=0;ii<aLen;ii++)
    LOG_OUTPUT("Sent 0x%1x",(sSerialOutBuffer[ii]));
  Viessmann::Datapoint::nextDatapoint();
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
    publishDataPoints();
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
       {
        sSerialOutBuffer[0] = Viessmann::COMM_INIT_1;
        sSerialOutBuffer[1] = Viessmann::COMM_INIT_2;
        sSerialOutBuffer[2] = Viessmann::COMM_INIT_3;
        Serial.write(sSerialOutBuffer,3);
        return aRet;
        }
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

    if (len>sizeof(sSerialBuffer)) len=sizeof(sSerialBuffer);
    sInputStringPos+= Serial.readBytes(&sSerialBuffer[sInputStringPos], len);

    LOG_INPUT("Start of in Dump################");
    for (int ii=0;ii<sInputStringPos;ii++)
      LOG_INPUT("%#1x",(sSerialBuffer[ii]));
    LOG_INPUT("Stop of in Dump-################");


    if (len ==1 && sSerialBuffer[0]==Viessmann::HERATBEAT)
    {
     sSerialOutBuffer[0] = Viessmann::COMM_INIT_1;
     sSerialOutBuffer[1] = Viessmann::COMM_INIT_2;
     sSerialOutBuffer[2] = Viessmann::COMM_INIT_3;
     Serial.write(sSerialOutBuffer,3);
     LOG_DEBUG("Sent COMM_INIT");
     sInputStringPos = 0;
    }
    else
    {
      Viessmann::Datapoint* aDataPoint=0;
      unsigned int aRet = Viessmann::Datapoint::dispatchData(sSerialBuffer,sInputStringPos,aDataPoint);
      if (aDataPoint)
      {
        LOG_DEBUG("Got: %s",aDataPoint->getName().c_str());
        LOG_DEBUG("Short Val is %d",aDataPoint->getValueAsShort());
        sInputStringPos = 0;
      }
      else
      {
        LOG_ERROR("Did not understand data (%d byte) ret Val is %d",sInputStringPos,aRet);
      }
    }


    if (sInputStringPos==BUFFER_SIZE)
      sInputStringPos=0;
  }
}

void setupOTA() {
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(4800,SERIAL_8E2);
  setup_wifi();
  setupOTA();

  sMqttClient.setServer(mqtt_server, 1883);
  sMqttClient.setCallback(onMqttData);

  Viessmann::Datapoint* aNewDataPoint = new Viessmann::Datapoint("TemperatureOutside",false,2,-60,100,0x5525);
  aNewDataPoint = new Viessmann::Datapoint("Kesseltemperatur",false,2,0,150,0x0810);
  Viessmann::Datapoint::resetIterator();
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
  ArduinoOTA.handle();
}


#else
#ifndef UNIT_TEST
int main (int argc, char* argv[]) {};
#endif
#endif
