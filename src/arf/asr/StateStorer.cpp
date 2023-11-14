#include "StateStorer.h"
#include "Logger.h"
#include "NwAid.h"
#include <errnoLib.h>



std::unique_ptr<StateStorer> StateStorer::CreateStateStorer(const unsigned int appId, const unsigned int port, std::string_view backupIp)
{
  std::unique_ptr<StateStorer> pSs{nullptr};
  
  
  std::unique_ptr<IReceiver> pRcv = NwAid::CreateUniCastReceiver(port);
  if(!pRcv)
  {
    LogMsg(LogPrioCritical, "ERROR: StateStorer::CreateStateStorer failed to create IReceiver (port %d). Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  
  std::unique_ptr<ISender> pSnd = NwAid::CreateUniCastSender(backupIp,port);
  if(!pSnd)
  {
    LogMsg(LogPrioCritical, "ERROR: StateStorer::CreateStateStorer failed to create ISender (port %d). Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
    
  
  pSs = std::make_unique<StateStorer>(appId,pRcv);
  if(!pSs)
  {
    LogMsg(LogPrioCritical, "ERROR: StateStorer::CreateStateStorer failed to create StateStorer. appId: %u  Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
  }
  
  return pSs;
}

StateStorer::StateStorer(const unsigned int appId,std::unique_ptr<ISender> &pSender, std::unique_ptr<IReceiver> &pReceiver) 
: m_appId{appId},m_pSender{std::move(pSender)},m_pReceiver{std::move(pReceiver)}
{
  
}



OSAStatusCode StateStorer::Start()
{
  static const int TaskPrio = 35;   
  static const std::string TaskNameBase{"tSs"};
  const std::string TaskName{TaskNameBase+std::to_string(m_appId)};
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)StateStorer::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
    LogMsg(LogPrioError, "ERROR: StateStorer::Start Failed to spawn FD task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return OSA_OK;
}

void StateStorer::Stop()
{
  LogMsg(LogPrioInfo, "StateStorer::Stop - Commanding StateStorer task to end. ");
  m_isRunning = false;
}



void StateStorer::RunStateMachine()
{
 
}


bool StateStorer::RcvMyStorageData()
{
 /* HeartbeatAFD incomingHeartbeat;
  while(m_pReceiver->Rcv(incomingHeartbeat))
  {
    if(incomingHeartbeat.GetSenderAppId() == m_appId)
    {
      return true;
    }
    else
    {
      LogMsg(LogPrioCritical, "AFD::RcvMyHb received heartbeat destined to another app. Rcvd: %u My: %u",incomingHeartbeat.GetSenderAppId(), m_appId);
    }
  }
  return false;*/
}

void StateStorer::Flush()
{
 // HeartbeatAFD incomingHeartbeat;
//  while(m_pReceiver->Rcv(incomingHeartbeat));
}



OSAStatusCode StateStorer::StorageTaskMethod()
{
  m_isRunning = true;
  LogMsg(LogPrioInfo, "ASR::StorageTaskMethod - ASR for AppId %u running ",m_appId);
  do
  {
    RunStateMachine();
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  
  LogMsg(LogPrioInfo, "WARNING: ASR::StorageTaskMethod - ASR for AppId %u terminated ",m_appId);
  return OSA_OK;
}


OSAStatusCode StateStorer::ClassTaskMethod(void * const pInstance)
{
  return ((StateStorer*)(pInstance))->StorageTaskMethod();
}


  
void StateStorer::Print() const
{
  LogMsg(LogPrioInfo,"--- >StateStorer< ---");
  LogMsg(LogPrioInfo,"AppId: %u",m_appId);
  LogMsg(LogPrioInfo,"--- <StateStorer> ---");
}



