#ifndef LOGGER_H
#define  LOGGER_H
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#define LOG(LVL, TEXT, ...) Logger::mqtt_log(LVL, __FILE__, __LINE__,TEXT,  ##__VA_ARGS__)
#define LOG_FATAL(TEXT,...) LOG(LOGLEVEL_FATAL, TEXT, ##__VA_ARGS__)
#define LOG_ERROR(TEXT,...) LOG(LOGLEVEL_ERROR, TEXT, ##__VA_ARGS__)
#define LOG_WARNING(TEXT,...) LOG(LOGLEVEL_WARNING, TEXT, ##__VA_ARGS__)
#define LOG_INFO(TEXT,...) LOG(LOGLEVEL_INFO, TEXT, ##__VA_ARGS__)
#define LOG_DEBUG(TEXT,...) LOG(LOGLEVEL_DEBUG, TEXT, ##__VA_ARGS__)
#define LOG_INPUT(TEXT,...) LOG(LOGLEVEL_INPUT,TEXT, ##__VA_ARGS__)
#define LOG_OUTPUT(TEXT,...) LOG(LOGLEVEL_OUTPUT, TEXT, ##__VA_ARGS__)
#define LOG_TOPIC "ESP8266LOG"
static const short LOGLEVEL_FATAL   = 0x00 << 0;
static const short LOGLEVEL_ERROR   = 0x01 << 0;
static const short LOGLEVEL_WARNING = 0x01 << 1;
static const short LOGLEVEL_INFO    = 0x01 << 2;
static const short LOGLEVEL_DEBUG   = 0x01 << 3;
static const short LOGLEVEL_INPUT   = 0x01 << 4;
static const short LOGLEVEL_OUTPUT  = 0x01 << 5;

class Logger {

public:

  typedef unsigned short logLevelT;

  static void mqtt_log( unsigned short theLevel, const char* theFile, unsigned short theLine, const char *theLogText, ...);
  static void setLogLevel(logLevelT theLogLevel) {msLogLevel=theLogLevel;};
  static void setClient(PubSubClient* thePubSub) {msPubSubClient = thePubSub;};

private:
  static logLevelT msLogLevel;
  Logger();
  static char msBuffer[512];
  static PubSubClient* msPubSubClient;

};
#endif
