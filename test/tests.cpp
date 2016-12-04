#include <unity.h>
#include <viessmann300.h>
#ifdef UNIT_TEST

void test_LogDebug(void);
void test_createTempRequest(void);
void test_readTempResponse(void);
void test_readKesselResponse(void);
void test_createBrennerRequest(void);
void test_readBrennerResponse(void);



int main(int argc, char **argv) {
    UNITY_BEGIN();    // IMPORTANT LINE!
    Viessmann::Datapoint* aNewDataPoint = new Viessmann::Datapoint("TemperatureOutside",false,2,-60,100,2,0x5525);
    aNewDataPoint = new Viessmann::Datapoint("Kesseltemperatur",false,2,0,150,0xFF,0x0810);
    aNewDataPoint = new Viessmann::Datapoint("BrennerStunden",false,4,0,1150000,0x0A0F0F0E,0x08A7);
    aNewDataPoint = new Viessmann::Datapoint("Warmwassertemperatur",false,2,0,150,0,0x0812);
    Viessmann::Datapoint::resetIterator();
    RUN_TEST(test_createTempRequest);
    RUN_TEST(test_readTempResponse);
    RUN_TEST(test_readKesselResponse);
    RUN_TEST(test_LogDebug);
    RUN_TEST(test_createBrennerRequest);
    RUN_TEST(test_readBrennerResponse);
    UNITY_END(); // stop unit testing
}
#endif
