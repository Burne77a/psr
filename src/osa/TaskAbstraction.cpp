#include "TaskAbstraction.h"
#include <taskLib.h>
#include <tickLib.h>
#include <errnoLib.h>

OSATaskId OSACreateTask(const std::string & name, const int prio, OSATaskFunction pEntryFunction,PTSInstancePtr instancePtr)
{
	static const int defaultStackSz = 40 * 1024;
	static const int defaultOptions = 0;
	const TASK_ID tskIdSender = taskSpawn((char*)name.c_str(), prio, defaultOptions, defaultStackSz,(FUNCPTR)pEntryFunction,(long)instancePtr,
				0,0,0,0,0,0,0,0,0);
	
	return (OSATaskId) tskIdSender;
}

void OSATaskSleep(const unsigned int waitTimeInMs)
{
	//assumes one ms tick
	taskDelay(waitTimeInMs);
}

unsigned int OSAGetMsCount()
{
	return tickGet();
}


