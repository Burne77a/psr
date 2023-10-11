#include "FD.h"    
#include "Logger.h"
#include <errnoLib.h>
FD::FD(GMM & gmm) : m_gmm(gmm) //Reference to the GMM class, could be changed to a interface to make the dependency injection better. 
{
  
}

FD::~FD()
{
  
}

OSAStatusCode FD::Start()
{
  static const int TaskPrio = 30;   
  static const std::string TaskName("tFdCcm");
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)FD::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
    LogMsg(LogPrioError, "ERROR FD::Start Failed to spawn CCM FD task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return OSA_OK;
}

void FD::Stop()
{
  LogMsg(LogPrioInfo, "FD::Stop() - Commanding FD task to end. ");
  m_isRunning = false;
}

void FD::PopulateAndSendHeartbeat()
{
  Populate();
  Send();
}

void FD::Populate()
{
  //vector with heartbeats, populated by the lambda function below
  m_gmm.ForEachMember([](int id, Member& member) 
  {
      member.
  });
}

void FD::Send()
{
  
}

void FD::HandleIncommingHeartbeat()
{
  
}
    

OSAStatusCode FD::FailureDetectionTaskMethod()
{
  m_isRunning = true;
  do
  {
    PopulateAndSendHeartbeat();
    HandleIncommingHeartbeat();
    
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  return OSA_OK;
}

OSAStatusCode FD::ClassTaskMethod(void * const pInstance)
{
  return ((FD*)(pInstance))->FailureDetectionTaskMethod();
}
