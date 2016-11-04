#include <unity.h>
#include <logger.h>
#ifdef UNIT_TEST
void test_LogDebug(void)
{
  PubSubClient aClient;
  Logger::setClient(&aClient);
  Logger::setLogLevel(LOGLEVEL_ERROR);
  LOG_DEBUG("This No %d should not be printed",1);
  Logger::setLogLevel(LOGLEVEL_DEBUG);
  LOG_DEBUG("This No %d should  be printed",2);
  TEST_ASSERT(true);
}

#endif
