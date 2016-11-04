#include "viessmann300.h"
#include <stdio.h>
using namespace Viessmann;

std::map<Datapoint::AddressT, Datapoint*> Datapoint::smDatapoints;

Datapoint::Datapoint(std::string theName,
          bool theWrite,
          unsigned int theLength,
          int theMinValue,
          int theMaxValue,
          AddressT theAdress):
    mName(theName),
    mWrite(theWrite),
    mLength(theLength),
    mMinValue(theMinValue),
    mMaxValue(theMaxValue),
    mAdress(theAdress)
    {
      smDatapoints[mAdress] = this;
      memset(mLastValue,0,MAX_VALUE_LENGTH);
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
          printf("Adress does not exist\n");
          return aPos;
        }

        smDatapoints[aAddress]->setValue(&theBuffer[aPos],aLength);
        aPos+=aLength;
        if (aLength!=smDatapoints[aAddress]->getLength())
        {
          printf("Len ist not machting %d != %d\n",aLength,smDatapoints[aAddress]->getLength());
          theDatapoint = 0;
          return aPos;
        }
        byte aChecksum = theBuffer[aPos++];
        theDatapoint = smDatapoints[aAddress];
        return aPos;
      }
      return aConsumed;
    }

    void Datapoint::setValue(const char* theDst, byte theLen)
    {
      if (theLen>MAX_VALUE_LENGTH)
        theLen=MAX_VALUE_LENGTH;
      //memcpy(mLastValue,theDst,theLen);
      for (int i=0;i<theLen;i++)
      {
        mLastValue[i] = theDst[i];
      }
    }

    const unsigned short Datapoint::getValueAsShort()
    {
       return *((short*)mLastValue);
    };

    int Datapoint::createReadRequest(char* theBuffer) const
    {
      unsigned int aPos=0;
      theBuffer[aPos++] = START_BYTE;
      theBuffer[aPos++] = 0; //skip length
      theBuffer[aPos++] = REQUST;
      theBuffer[aPos++] = READ;
      theBuffer[aPos+=sizeof(mAdress)] = mAdress;
      byte aCheckSum = 0;
      for (unsigned int aCsPos=1; aCsPos<aPos;aCsPos++)
      {
          aCheckSum	+=theBuffer[aCsPos];
      }
      theBuffer[aPos] = aCheckSum;
      theBuffer[LEN_POS] = aPos-1;
      return aPos+1;


    }
