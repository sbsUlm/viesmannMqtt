#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "bwlan"
#define wifi_password "8594899141610047"
#define mqtt_server "sbspi"
#define ROOT_TOPIC heizung
#define STATUS_TOPIC "ROOT_TOPIC/Status"
#define HEARTBEAT_TOPIC "ROOT_TOPIC/Heartbeat"
#define SSID_TOPIC "/WIFI/SSID"
#define IP_TOPIC "/WIFI/IP"
#define CMD_TOPIC "ROOT_TOPIC/Command"
#define SERIAL_TOPIC "ROOT_TOPIC/Serial"
#define BUFFER_SIZE 1
#define HEARTBEAT 10000 //seconds between keepalive msgs
#define SLEEP_TIME 10
char aBuffer[BUFFER_SIZE];
char aSerialBuffer[BUFFER_SIZE];

void setup_wifi();


WiFiClient espClient;
PubSubClient client(espClient);

void onMqttData(char* topic, byte* payload, unsigned int length) {
  memset(aBuffer,0,BUFFER_SIZE);
#ifdef DEBUG
  snprintf(aBuffer,BUFFER_SIZE,"Callback for %s received:", topic);
  Serial.println(aBuffer);
    // create character buffer with ending null terminator (string)
  if (length > BUFFER_SIZE-1)
   length = BUFFER_SIZE-1;
  memcpy(aBuffer,payload,length);
  aBuffer[length] = '\0';
  Serial.println(aBuffer);
#endif
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(onMqttData);
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
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe(CMD_TOPIC);
      snprintf(aBuffer,BUFFER_SIZE,"%s%s",STATUS_TOPIC,SSID_TOPIC);
      client.publish(aBuffer,wifi_ssid,true);
      snprintf(aBuffer,BUFFER_SIZE,"%s%s",STATUS_TOPIC,IP_TOPIC);
      client.publish(aBuffer,WiFi.localIP().toString().c_str(),true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

long lastMsg = 0;
long heartbeatCounter = 0;
void heartbeat()
{
  long now = millis();
  if (now - lastMsg > HEARTBEAT) {
    lastMsg = now;
    snprintf(aBuffer,BUFFER_SIZE,"Uptime %d",(heartbeatCounter*(HEARTBEAT/1000)));
    client.publish(HEARTBEAT_TOPIC, aBuffer, false);
    }
}

int inputStringPos = 0;
void serialRead()
{
  size_t len = Serial.available();
  if(len){
    inputStringPos+= Serial.readBytes(&aSerialBuffer[inputStringPos], len);
  }
  if (inputStringPos > 10) {
    Serial.println("Rxd Data");
    aSerialBuffer[inputStringPos+1] = '\0';
    Serial.println(aSerialBuffer);
    client.publish(SERIAL_TOPIC, aSerialBuffer, false);
    memset(aSerialBuffer, 0, BUFFER_SIZE);
    inputStringPos = 0;
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  heartbeat();
  serialRead();
  ESP.deepSleep(SLEEP_TIME * 1000000);
}
