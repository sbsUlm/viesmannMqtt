#ifndef DEBUG_OUT_H
#define DEBUG_OUT_H
//#define DEBUG_LOGGING

#ifdef DEBUG_LOGGING
#ifdef ESP8266
#define DEBUG_OUT(WHAT)  Serial.println(WHAT)
#else
#define DEBUG_OUT(WHAT)  printf(WHAT); printf("\n")
#endif
#else
#define DEBUG_OUT(WHAT) do { } while(0)
#endif

#endif
