#include "viessmann300.h"
#include "logger.h"
#include <stdio.h>
using namespace Viessmann;

std::map<Datapoint::AddressT, Datapoint*> Datapoint::smDatapoints;
Datapoint::DatapointIteratorT Datapoint::smDataPointInterator;

Datapoint::Datapoint(std::string theName,
          bool theWrite,
          unsigned int theLength,
          int theMinValue,
          int theMaxValue,
          int theDefaultValue,
          AddressT theAdress):
    mName(theName),
    mWrite(theWrite),
    mLength(theLength),
    mMinValue(theMinValue),
    mMaxValue(theMaxValue),

    mAdress(theAdress)
    {
      memset(mLastValue,0,MAX_VALUE_LENGTH);
      (*(int*)mLastValue)=theDefaultValue;
      mLastValue[0] = theDefaultValue       & 0xFF;
      mLastValue[1] = (theDefaultValue>>8)  & 0xFF;
      mLastValue[2] = (theDefaultValue>>16) & 0xFF;
      mLastValue[3] = (theDefaultValue>>24) & 0xFF;
            printf("in dp ctor %d\n",__LINE__);
      smDatapoints[mAdress] = this;
    }

    Datapoint::~Datapoint()
    {
      smDatapoints.erase(mAdress);
    }

    int  Datapoint::dispatchData(const char* theBuffer,
                  const unsigned int theLen,
                  Datapoint*& theDatapoint)
    {
      theDatapoint=0;
      unsigned int aPos=0;
      unsigned int aConsumed=0;
      bool aStartByteFound=false;
      while (aPos<theLen)
      {
        if (aStartByteFound == false)
          if (theBuffer[aPos++] != START_BYTE)
          {
            continue;
          }
          else
          {
            aStartByteFound=true;
            aConsumed=aPos;
            continue;
          }
        byte aDataLength=theBuffer[aPos++];
          if (theLen<aDataLength+4)
            return aConsumed;
        byte aType1 = theBuffer[aPos++];
        byte aType2 = theBuffer[aPos++];
        unsigned short aAddress;
        ((char*)(&aAddress))[1]=theBuffer[aPos++];
        ((char*)(&aAddress))[0]=theBuffer[aPos++];
        byte aLength = theBuffer[aPos++];
        if (smDatapoints[aAddress] ==0)
        {
          LOG_ERROR("Adress does not exist\n");
          return aPos;
        }
        LOG_DEBUG("Adress ist 0x%x",aAddress);
        LOG_DEBUG("Datapoint is %s",smDatapoints[aAddress]->getName().c_str());
        smDatapoints[aAddress]->setValue(&theBuffer[aPos],aLength);
        aPos+=aLength;
        if (aLength!=smDatapoints[aAddress]->getLength())
        {
          LOG_ERROR("Len ist not machting %d != %d\n",aLength,smDatapoints[aAddress]->getLength());
          theDatapoint = 0;
          return aPos;
        }
        byte aChecksum = theBuffer[aPos++];
        theDatapoint = smDatapoints[aAddress];
        LOG_DEBUG("Value is %d",theDatapoint->getValueAsInt());
        return aPos;
      }
      return aConsumed;
    }

    void Datapoint::setValue(const char* theDst, byte theLen)
    {
      LOG_INPUT("Setting value of %s with %d bytes",this->getName().c_str(), theLen);
      if (theLen>MAX_VALUE_LENGTH)
        theLen=MAX_VALUE_LENGTH;
      //memcpy(mLastValue,theDst,theLen);
      for (int i=0;i<theLen;i++)
      {
        LOG_INPUT("Byte %d is 0x%x", i,theDst[i]);
        mLastValue[i] = theDst[i];
      }
      LOG_INPUT("Value of %s is %d",this->getName().c_str(),this->getValueAsInt());
    }

    const unsigned short Datapoint::getValueAsShort() const
    {
       return *((short*)mLastValue);
    };

    const unsigned short Datapoint::getValueAsByte() const
    {
       unsigned short aShort = 0;
       aShort = mLastValue[0];
       return aShort;
    };

    const unsigned int Datapoint::getValueAsInt() const
    {
       unsigned int aInt = 0;
       aInt |= ( mLastValue[0] & 0xFF);
       aInt |= ((mLastValue[1] & 0xFF) <<8);
       aInt |= ((mLastValue[2] & 0xFF) <<16);
       aInt |= ((mLastValue[3] & 0xFF) <<24);
       return aInt;
    };

    void Datapoint::nextDatapoint()
    {
      smDataPointInterator++;
      if (smDataPointInterator==smDatapoints.end())
        resetIterator();
    };

    int Datapoint::createReadRequest(char* theBuffer) const
    {
      unsigned int aPos=0;
      theBuffer[aPos++] = START_BYTE;
      theBuffer[aPos++] = 0; //skip length
      theBuffer[aPos++] = REQUST;
      theBuffer[aPos++] = READ;
      theBuffer[aPos++] = (mAdress >> 8) & 0xFF;
      theBuffer[aPos++] = mAdress & 0xFF;
      theBuffer[aPos++] = mLength;
      theBuffer[LEN_POS] = aPos-2;
      byte aCheckSum = 0;
      for (unsigned int aCsPos=1; aCsPos<aPos;aCsPos++)
      {
          aCheckSum	+=theBuffer[aCsPos];
      }
      theBuffer[aPos] = aCheckSum;

      return aPos+1;


    }
