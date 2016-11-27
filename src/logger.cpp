#include "logger.h"
#include <cstdio>
#include <cstdarg>
#include "debugOut.h"
unsigned short Logger::msLogLevel = (LOGLEVEL_FATAL | LOGLEVEL_ERROR | LOGLEVEL_INFO | LOGLEVEL_WARNING );
char Logger::msBuffer[512];
char Logger::msTextBuffer[256];
PubSubClient* Logger::msPubSubClient=0;

PubSubClient* msPubSubClient;
void Logger::mqtt_log(unsigned short theLevel, const char* theFile, unsigned short theLine, const char *theLogText, ...)
{
  if ( (msLogLevel & theLevel) == 0 )
    return;
    va_list arg_list;
    va_start(arg_list, theLogText);
    const size_t needed = vsnprintf(msTextBuffer, sizeof msTextBuffer,
                                        theLogText, arg_list) + 1;
  snprintf(msBuffer, sizeof(msBuffer),"%d: %s, %s, %d",
    theLevel, msTextBuffer, theFile, theLine);
  if (msPubSubClient)
    msPubSubClient->publish(LOG_TOPIC, msBuffer, false);
  DEBUG_OUT(msBuffer);
}
