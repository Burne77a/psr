#ifndef OSFUNCTIONS_TASKABSTRACTION_H_
#define OSFUNCTIONS_TASKABSTRACTION_H_

#include "ErrorCodes.h"
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

typedef long OSATaskId;

typedef int (*OSATaskFunction)(); 

typedef void * OSAInstancePtr;


OSATaskId OSACreateTask(const std::string & name, const int prio, OSATaskFunction pEntryFunction,OSAInstancePtr instancePtr);

void OSATaskSleep(const uint32_t waitTimeInMs);

unsigned int OSAGetMsCount();



#ifdef __cplusplus
}
#endif



#endif
