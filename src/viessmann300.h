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
  static const byte COMM_INIT_1 = 0x16;
  static const byte COMM_INIT_2 = 0x00;
  static const byte COMM_INIT_3 = 0x00;
  static const byte START_BYTE = 0x41;
  static const byte REQUST = 0x00;
  static const byte RESPONSE = 0x01;
  static const byte ERROR = 0x03;
  static const byte READ = 0x01;
  static const byte WRITE = 0x02;
  static const byte CALL = 0x07;
  static const int LEN_POS = 1;
  static const int MAX_VALUE_LENGTH = 8;

  class Datapoint
  {
    public:

      typedef unsigned short AddressT;
      typedef std::map<AddressT, Datapoint*>::iterator DatapointIteratorT;

    Datapoint(std::string theName,
              bool theWrite,
              unsigned int theLength,
              int theMinValue,
              int theMaxValue,
              int theDefaultValue,
              AddressT theAdress);

      ~Datapoint();

      int createReadRequest(char* theBuffer) const;

      static int dispatchData(const char* theBuffer,
                              const unsigned int theLen,
                              Datapoint*& theDatapoint);

      void setName(std::string theName) {this->mName=theName;};
      const std::string getName() const {return this->mName;};
      const short getAddress() const {return this->mAdress;};
      const unsigned int getLength() const {return mLength;};

      void setValue(const char* theDst, byte theLen);

      const unsigned short getValueAsByte() const;
      const unsigned short getValueAsShort() const;
      const unsigned int getValueAsInt() const;

      static const Datapoint* getDatapoint(AddressT theAddress)
        {return smDatapoints[theAddress];};

      static DatapointIteratorT getIterator()
        {return smDataPointInterator;};

      static void resetIterator()
        {smDataPointInterator = smDatapoints.begin();};

      static DatapointIteratorT getDatapointIt()
        {return smDatapoints.begin();};

        static DatapointIteratorT getEndIt()
          {return smDatapoints.end();};

      static void nextDatapoint();

      private:
        std::string mName;
        bool mWrite;
        unsigned int mLength;
        int mMinValue;
        int mMaxValue;
        AddressT mAdress;
        static std::map<AddressT, Datapoint*> smDatapoints;
        char mLastValue[MAX_VALUE_LENGTH];
        static DatapointIteratorT smDataPointInterator;
  };


}
#endif
