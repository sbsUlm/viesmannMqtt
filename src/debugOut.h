#ifndef DEBUG_OUT_H
#define DEBUG_OUT_H

#ifdef DEBUG
#define DEBUG_OUT(WHAT)  Serial.println(WHAT)
#else
#define DEBUG_OUT(WHAT) do { } while(0)
#endif

#endif
