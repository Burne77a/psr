#ifndef OSFUNCTIONS_TASKABSTRACTION_H_
#define OSFUNCTIONS_TASKABSTRACTION_H_

#include "ErrorCodes.h"
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

typedef long OSATaskId;

typedef int (*OSATaskFunction)(); 

typedef void * PTSInstancePtr;


OSATaskId OSACreateTask(const std::string & name, const int prio, OSATaskFunction pEntryFunction,PTSInstancePtr instancePtr);

void OSATaskSleep(const unsigned int waitTimeInMs);

unsigned int OSAGetMsCount();



#ifdef __cplusplus
}
#endif



#endif
