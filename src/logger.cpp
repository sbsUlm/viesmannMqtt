#include "logger.h"

unsigned short Logger::msLogLevel = (LOGLEVEL_FATAL | LOGLEVEL_ERROR | LOGLEVEL_INFO | LOGLEVEL_WARNING);
char Logger::msBuffer[512];
PubSubClient* Logger::msPubSubClient=0;

PubSubClient* msPubSubClient;
void Logger::mqtt_log(unsigned short theLevel, const char* theFile, unsigned short theLine, const char *theLogText, ...)
{
  if ( (msLogLevel & theLevel) == 0 )
    return;

  snprintf(msBuffer, sizeof(msBuffer),"%d: %s, %s, %d",
    theLevel, theLogText, theFile, theLine);
  if (msPubSubClient)
    msPubSubClient->publish(LOG_TOPIC, msBuffer, false);
}
