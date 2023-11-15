#include "StateKeeper.h"
#include "Logger.h"
#include "NwAid.h"
#include <errnoLib.h>



std::unique_ptr<StateKeeper> StateKeeper::CreateStateKeeper(const unsigned int appId, const unsigned int port, std::string_view backupIp)
{
  std::unique_ptr<StateKeeper> pSk{nullptr};
  
  
  std::unique_ptr<IReceiver> pRcv = NwAid::CreateUniCastReceiver(port);
  if(!pRcv)
  {
    LogMsg(LogPrioCritical, "ERROR: StateKeeper::CreateStateKeeper failed to create IReceiver (port %d). Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  
  std::unique_ptr<ISender> pSnd = NwAid::CreateUniCastSender(backupIp,port);
  if(!pSnd)
  {
    LogMsg(LogPrioCritical, "ERROR: StateKeeper::CreateStateKeeper failed to create ISender (port %d). Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
    
  
  pSk = std::make_unique<StateKeeper>(appId,pSnd,pRcv);
  if(!pSk)
  {
    LogMsg(LogPrioCritical, "ERROR: StateKeeper::CreateStateKeeper failed to create StateKeeper. appId: %u  Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
  }
  
  return pSk;
}

StateKeeper::StateKeeper(const unsigned int appId,std::unique_ptr<ISender> &pSender, std::unique_ptr<IReceiver> &pReceiver) 
: m_appId{appId},m_pSender{std::move(pSender)},m_pReceiver{std::move(pReceiver)}
{
  
}



OSAStatusCode StateKeeper::Start()
{
  static const int TaskPrio = 35;   
  static const std::string TaskNameBase{"tSs"};
  const std::string TaskName{TaskNameBase+std::to_string(m_appId)};
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)StateKeeper::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
    LogMsg(LogPrioError, "ERROR: StateKeeper::Start Failed to spawn FD task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return OSA_OK;
}

void StateKeeper::Stop()
{
  LogMsg(LogPrioInfo, "StateKeeper::Stop - Commanding StateStorer task to end. ");
  m_isRunning = false;
}



void StateKeeper::RunStateMachine()
{
  RcvMyStorageData();
}

void StateKeeper::StoreState(StateDataMsg &stateDataMsg)
{  
  if(!m_pLatestStateDataMsg)
  {
    m_pLatestStateDataMsg = std::make_unique<StateDataMsg>(stateDataMsg);
  }
  else
  {
    *m_pLatestStateDataMsg = stateDataMsg;
  }
}

void StateKeeper::SendLatestStateToRequester(StateDataMsg &stateDataMsg)
{
  if(m_pLatestStateDataMsg)
  {
    StateDataMsg reply(StateDataMsg::MsgType::Fetched,stateDataMsg);
    if(!m_pSender->Send(reply))
    {
      LogMsg(LogPrioCritical, "WARNING: StateKeeper::SendLatestStateToRequester - failed to send latest state to backup %u",m_appId);
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "WARNING: StateKeeper::SendLatestStateToRequester - no state to send AppId %u",m_appId);
  }
}

void StateKeeper::RcvMyStorageData()
{
  StateDataMsg incomingStateData;
  bool isMsgRcvd = false;
  while(m_pReceiver->Rcv(incomingStateData))
  {
    if(incomingStateData.GetAppId() == m_appId)
    {
      if(incomingStateData.IsStoreMsg())
      {
        StoreState(incomingStateData);
      }
      else if(incomingStateData.IsGetMsg())
      {
        SendLatestStateToRequester(incomingStateData);
      }
    }
    else
    {
      LogMsg(LogPrioCritical, "StateKeeper::RcvMyStorageData received state data destined to another app. Rcvd: %u My: %u",incomingStateData.GetAppId(), m_appId);
    }
  }

}

void StateKeeper::Flush()
{
  StateDataMsg incomingStateData;
  while(m_pReceiver->Rcv(incomingStateData));
}



OSAStatusCode StateKeeper::StorageTaskMethod()
{
  m_isRunning = true;
  LogMsg(LogPrioInfo, "StateKeeper::StorageTaskMethod - StateStorer for AppId %u running ",m_appId);
  do
  {
    RunStateMachine();
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  
  LogMsg(LogPrioInfo, "WARNING: StateKeeper::StorageTaskMethod - StateStorer for AppId %u terminated ",m_appId);
  return OSA_OK;
}


OSAStatusCode StateKeeper::ClassTaskMethod(void * const pInstance)
{
  return ((StateKeeper*)(pInstance))->StorageTaskMethod();
}


  
void StateKeeper::Print() const
{
  LogMsg(LogPrioInfo,"--- >StateKeeper< ---");
  LogMsg(LogPrioInfo,"AppId: %u StateReceived: %s",m_appId, m_pLatestStateDataMsg ? "YES": "NO");
  if(m_pLatestStateDataMsg)
  {
    m_pLatestStateDataMsg->Print();
  }
  LogMsg(LogPrioInfo,"--- <StateKeeper> ---");
}



