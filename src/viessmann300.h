#ifndef VIESMANN_300_H
#define VIESMANN_300_H
//#include <Arduino.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <map>
namespace Viessmann
{
typedef char byte;
  static const byte HERATBEAT = 0x05;
  static const byte INIT = 0x04;
  static const int COMM_INIT = 0x160000;
  static const byte START_BYTE = 0x41;
  static const byte REQUST = 0x00;
  static const byte RESPONSE = 0x01;
  static const byte ERROR = 0x03;
  static const byte READ = 0x01;
  static const byte WRITE = 0x02;
  static const byte CALL = 0x07;
  static const int LEN_POS = 1;
  static const int MAX_VALUE_LENGTH = 4;

  class Datapoint
  {
    public:

      typedef unsigned short AddressT;

    Datapoint(std::string theName,
              bool theWrite,
              unsigned int theLength,
              int theMinValue,
              int theMaxValue,
              AddressT theAdress);


      int createReadRequest(char* theBuffer);

      static int dispatchData(const char* theBuffer,
                              const unsigned int theLen,
                              Datapoint*& theDatapoint);

      void setName(std::string theName) {this->mName=theName;};

      const unsigned int getLength() {return mLength;};

      void setValue(const char* theDst, byte theLen);

      const unsigned short getValueAsShort();

      private:
        std::string mName;
        bool mWrite;
        unsigned int mLength;
        int mMinValue;
        int mMaxValue;
        AddressT mAdress;
        static std::map<AddressT, Datapoint*> smDatapoints;
        char mLastValue[MAX_VALUE_LENGTH];
  };


}
#endif
