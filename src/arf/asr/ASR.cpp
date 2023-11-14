#include "ASR.h"
#include "Logger.h"
#include "NwAid.h"
#include <errnoLib.h>

static const int APP_STORE_PORT_BASE = 9500;

std::unique_ptr<ASR> ASR::CreateASR(const unsigned int appId)
{
  const int appStatePort = APP_STORE_PORT_BASE + appId;
  
  std::unique_ptr<ASR> pAsr{nullptr};
  
  
  std::unique_ptr<IReceiver> pRcv = NwAid::CreateUniCastReceiver(appStatePort);
  if(!pRcv)
  {
    LogMsg(LogPrioCritical, "ERROR: ASR::CreateASR failed to create IReceiver (port %d). Errno: 0x%x (%s)",appStatePort,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
    
  
  pAsr = std::make_unique<ASR>(appId,pRcv);
  if(!pAsr)
  {
    LogMsg(LogPrioCritical, "ERROR: ASR::CreateASR failed to create ASR. appId: %u  Errno: 0x%x (%s)",appId,errnoGet(),strerror(errnoGet()));
  }
  
  return pAsr;
}

ASR::ASR(const unsigned int appId, std::unique_ptr<IReceiver> &pReceiver) 
: m_appId{appId}, m_pSender{std::move(pSender)},m_pReceiver{std::move(pReceiver)}
{
  
}



OSAStatusCode ASR::Start()
{
  static const int TaskPrio = 35;   
  static const std::string TaskNameBase{"tASR"};
  const std::string TaskName{TaskNameBase+std::to_string(m_appId)};
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)ASR::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
    LogMsg(LogPrioError, "ERROR: ASR::Start Failed to spawn FD task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return OSA_OK;
}

void ASR::Stop()
{
  LogMsg(LogPrioInfo, "ASR::Stop() - Commanding ASD task to end. ");
  m_isRunning = false;
}



void ASR::RunStateMachine()
{
 
}


bool ASR::RcvMyStorageData()
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

void ASR::Flush()
{
 // HeartbeatAFD incomingHeartbeat;
//  while(m_pReceiver->Rcv(incomingHeartbeat));
}



OSAStatusCode ASR::StorageTaskMethod()
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


OSAStatusCode ASR::ClassTaskMethod(void * const pInstance)
{
  return ((ASR*)(pInstance))->StorageTaskMethod();
}


  
void ASR::Print() const
{
  LogMsg(LogPrioInfo,"--- >ASR< ---");
  LogMsg(LogPrioInfo,"AppId: %u",m_appId);
  LogMsg(LogPrioInfo,"--- <ASR> ---");
}



