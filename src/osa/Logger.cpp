#include "Logger.h"
#include <syslog.h>
#include <stdarg.h>
#include <ioLib.h>
#include <shellLib.h>

const int LogPrioEmergency =  0;
const int LogPrioAlert =  1;
const int LogPrioCritical =  2;
const int LogPrioError =  3;
const int LogPrioWarning = 4;
const int LogPrioInfo =  5;

void LogMsgInit()
{
	SHELL_ID shellId= shellFirst();
	
	int inFd,outFd,errFd;
	shellIOStdGet(shellId,&inFd,&outFd,&errFd);
	
	while((shellId = shellNext(shellId)) != 0)
	{
		shellIOStdGet(shellId,&inFd,&outFd,&errFd);
	};
	
	syslogFdSet (outFd);
}

void LogMsg(const int priority, const char * message, ...)
{
	va_list args;
	va_start(args,message);
	vsyslog(priority,message,args);
	va_end(args);
}


