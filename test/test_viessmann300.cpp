#include "viessmann300.h"
#include <unity.h>

#ifdef UNIT_TEST


void test_createTempRequest(void)
{
  const Viessmann::Datapoint* aDataPoint;//("Temp",false,2,-60,100,0x5525);
  aDataPoint = Viessmann::Datapoint::getDatapoint(0x5525);
  char aChar[256];
  aDataPoint->createReadRequest(aChar);
  char aExp[8] = {0x41, 0x05, 0x00, 0x01, 0x55, 0x25, 0x02, 0x82};
  TEST_ASSERT_EQUAL_STRING_LEN(aExp, aChar, 8);
}

void test_readTempResponse(void)
{
  Viessmann::Datapoint* aDataPoint=0;

  char aChar[11] = {0x06, 0x41, 0x07, 0x01, 0x01, 0x55, 0x25, 0x02, 0x07, 0x01, 0x8D};
  unsigned int aRet = Viessmann::Datapoint::dispatchData(aChar,11,aDataPoint);
  unsigned short aShort=0;
  if (aDataPoint)
    aShort = aDataPoint->getValueAsShort();
  else
      TEST_ASSERT_TRUE(false);
  TEST_ASSERT_EQUAL_INT16(11,aRet);
  TEST_ASSERT_EQUAL_INT16(263,aShort);
  TEST_ASSERT_EQUAL_STRING("TemperatureOutside", aDataPoint->getName().c_str());

}

void test_readKesselResponse(void)
{
  Viessmann::Datapoint* aDataPoint=0;

  char aChar[11] = {0x06, 0x41, 0x07, 0x01, 0x01, 0x08, 0x10, 0x02, 0x64, 0x00, 0x8D};
  unsigned int aRet = Viessmann::Datapoint::dispatchData(aChar,11,aDataPoint);
  unsigned short aShort=0;
  if (aDataPoint)
    aShort = aDataPoint->getValueAsShort();
  else
      TEST_ASSERT_TRUE(false);
  TEST_ASSERT_EQUAL_INT16(11,aRet);
  TEST_ASSERT_EQUAL_INT16(100,aShort);
  TEST_ASSERT_EQUAL_STRING("Kesseltemperatur", aDataPoint->getName().c_str());

}

int main(int argc, char **argv) {
    UNITY_BEGIN();    // IMPORTANT LINE!
    Viessmann::Datapoint aNewDataPoint("TemperatureOutside",false,2,-60,100,0x5525);
    Viessmann::Datapoint aNewDataPoint2("Kesseltemperatur",false,2,0,150,0x0810);
    RUN_TEST(test_createTempRequest);
    RUN_TEST(test_readTempResponse);
    RUN_TEST(test_readKesselResponse);
    UNITY_END(); // stop unit testing
}

#endif
