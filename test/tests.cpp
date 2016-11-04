#include <unity.h>
#include <viessmann300.h>
#ifdef UNIT_TEST

void test_LogDebug(void);
void test_createTempRequest(void);
void test_readTempResponse(void);
void test_readKesselResponse(void);


int main(int argc, char **argv) {
    UNITY_BEGIN();    // IMPORTANT LINE!
    Viessmann::Datapoint aNewDataPoint("TemperatureOutside",false,2,-60,100,0x5525);
    Viessmann::Datapoint aNewDataPoint2("Kesseltemperatur",false,2,0,150,0x0810);
    RUN_TEST(test_createTempRequest);
    RUN_TEST(test_readTempResponse);
    RUN_TEST(test_readKesselResponse);
    RUN_TEST(test_LogDebug);
    UNITY_END(); // stop unit testing
}
#endif
