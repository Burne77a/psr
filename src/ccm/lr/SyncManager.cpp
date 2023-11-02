#include "SyncManager.h"
#include "NwAid.h"
#include "../misc/Misc.h"
#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<SyncManager> SyncManager::CreateSyncManager(GMM & gmm,ReplicatedLog & replicatedLog)
{
  static const int LOG_SYNC_PORT = 8888;
  
  std::vector<std::unique_ptr<ISender>> senders;
  bool isAllCreationOk = Misc::CreateISendersFromMembersExcludingMySelf(LOG_SYNC_PORT,gmm,senders);
  
  if(!isAllCreationOk)
  {
    LogMsg(LogPrioCritical, "ERROR: SyncManager::CreateSyncManager failed to create sender(s). Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  else
  {
    std::unique_ptr<IReceiver> pRcv = NwAid::CreateUniCastReceiver(LOG_SYNC_PORT);
    if(!pRcv)
    {
      LogMsg(LogPrioCritical, "ERROR: SyncManager::CreateSyncManager failed to create receiver. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
      isAllCreationOk = false;
    }
    
    if(isAllCreationOk)
    {
      std::unique_ptr<SyncManager> pSm = std::make_unique<SyncManager>(gmm,replicatedLog,senders,pRcv);
      if(!pSm)
      {
        LogMsg(LogPrioCritical, "ERROR: SyncManager::CreateSyncManager failed to create SyncManager.");
      }
      else
      {
        LogMsg(LogPrioInfo, "SyncManager::CreateSyncManager Successfully created a SyncManager instance.");
        return pSm;
      }
      
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: SyncManager::CreateSyncManager failed to create sender.");
    }
  } 
  return nullptr;
}

SyncManager::SyncManager(GMM & gmm, ReplicatedLog & replicatedLog, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver) : m_gmm(gmm), //Reference to the GMM class, could be changed to a interface to make the dependency injection better. 
m_replicatedLog{replicatedLog},m_senders(),m_pReceiver(std::move(pReceiver))
{
  for(auto & pSender : senders)
  {
    m_senders.push_back(std::move(pSender));
  }
}

SyncManager::~SyncManager()
{
  Stop();
}

OSAStatusCode SyncManager::Start()
{
  static const int TaskPrio = 30;   
  static const std::string TaskName("tSyncMg");
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)SyncManager::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
    LogMsg(LogPrioError, "ERROR SyncManager::Start Failed to spawn LR SyncManager task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return OSA_OK;
}

void SyncManager::Stop()
{
  LogMsg(LogPrioInfo, "SyncManager::Stop() - Commanding SyncManager task to end. ");
  m_isRunning = false;
}

void SyncManager::ServeIncomingSyncRequests(const SyncMsg &intiatingMsg)
{
  const std::string requesterIp = m_gmm.GetIp(intiatingMsg.GetIdOfRequester());
  std::vector<std::shared_ptr<SyncMsg>> entriesToSend;
  if(m_replicatedLog.GetLogEntriesAsSyncMsgVector(m_gmm.GetMyId(),entriesToSend))
  {
    SendEntriesToRequester(requesterIp,entriesToSend);
  }
  else
  {
    LogMsg(LogPrioError, "SyncManager::ServeIncomingSyncRequests() - Failed to get log entries in a transmittable format ");
  }  
}

void SyncManager::HandleIncomingSyncRequestFromOthers()
{
  SyncMsg incomingSyncMsg;
  while(RcvMsg(*m_pReceiver,incomingSyncMsg))
  {
    if(incomingSyncMsg.IsRequest())
    {
      LogMsg(LogPrioInfo, "SyncManager::HandleInCommingSyncRequestFromOthers() - Sync request received.");
      incomingSyncMsg.Print();
      ServeIncomingSyncRequests(incomingSyncMsg);
    }
    else
    {
      LogMsg(LogPrioWarning, "SyncManager::HandleInCommingSyncRequestFromOthers() - Received other than request, when expecting request. Ignoring ");
      incomingSyncMsg.Print();
    }
  }
}


void SyncManager::IssueRequestToSyncToInstanceWithLargestOp()
{
  const int idToReqFrom = m_gmm.GetIdOfAliveMemberWithOpNumberEqual(m_gmm.GetLargestOpNumberGossipedAndMySelf());
  if(idToReqFrom != INVALID_ID)
  {
    if(idToReqFrom != m_gmm.GetMyId())
    {
      const std::string ipToInstanceToReqFrom = m_gmm.GetIp(idToReqFrom);
      SendRequestForLog(ipToInstanceToReqFrom);
      WaitForAndProcessIncomingLogEntries();
    }
    else
    {
      LogMsg(LogPrioWarning, "SyncManager::IssueRequestToSyncToInstanceWithLargestOp() - ID of instance with highest OP number is myself. Do nothing.  ");
    }
  }
  else
  {
    LogMsg(LogPrioWarning, "SyncManager::IssueRequestToSyncToInstanceWithLargestOp() - No instance found alive to request from.  ");
  }
}

void SyncManager::HandleIncomingSyncMsg(bool &isComplete, std::vector<std::shared_ptr<SyncMsg>> & rcvdSyncMessages, bool &isToAbortDueToFailure, const SyncMsg &incomingMsg)
{
  unsigned int expectedNextPacket = 0U;
  if(rcvdSyncMessages.size() > 0)
  {
    std::shared_ptr<SyncMsg> pPrevMsg = rcvdSyncMessages[rcvdSyncMessages.size() - 1];
    if(pPrevMsg)
    {
      expectedNextPacket = pPrevMsg->GetCurrentIndex() + 1;
    }
    else
    {
      LogMsg(LogPrioWarning, "SyncManager::HandleIncomingSyncMsg() - Invalid pointer in rcvdSyncMessages %d ",rcvdSyncMessages.size() );
      return;
    }
  }
  if(incomingMsg.GetCurrentIndex() != expectedNextPacket)
  {
    LogMsg(LogPrioWarning, "SyncManager::HandleIncomingSyncMsg() - received wrong sequence %u %u ",incomingMsg.GetCurrentIndex(),expectedNextPacket );
    isToAbortDueToFailure = true;
    return;
  }
  
  if(incomingMsg.GetCurrentIndex() >= incomingMsg.GetTotalNumberOfEntries())
  {
    LogMsg(LogPrioWarning, "SyncManager::HandleIncomingSyncMsg() - received too high sequnce number %u %u ",incomingMsg.GetCurrentIndex(),incomingMsg.GetTotalNumberOfEntries() );
    isToAbortDueToFailure = true;
    return;
  }
  std::shared_ptr<SyncMsg> pSm = std::make_shared<SyncMsg>(incomingMsg);
  if(!pSm)
  {
    LogMsg(LogPrioError, "ERROR SyncManager::HandleIncomingSyncMsg Create SyncMsg Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    isToAbortDueToFailure = true;
    return;
  }
  rcvdSyncMessages.push_back(pSm);
  
  if(rcvdSyncMessages.size() == incomingMsg.GetTotalNumberOfEntries())
  {
    LogMsg(LogPrioInfo, "SyncManager::HandleIncomingSyncMsg All messages received for sync");
    isComplete = true;
  }
  
}

void SyncManager::WaitForAndProcessIncomingLogEntries()
{
  bool isDoneOrAborted = false;
  static const std::chrono::milliseconds TimeToWaitInMsDuration (10000);
  std::vector<std::shared_ptr<SyncMsg>> rcvdSyncMessages;
  auto startOfWait = std::chrono::system_clock::now();
  do
  {
    SyncMsg incomingMsg;
    if(RcvMsg(*m_pReceiver, incomingMsg))
    {
      if(incomingMsg.IsEntry())
      {
        bool isCompleted = false;
        bool isToAbort = false;
        HandleIncomingSyncMsg(isCompleted,rcvdSyncMessages,isToAbort,incomingMsg);
        if(isToAbort)
        {
          LogMsg(LogPrioWarning, "SyncManager::WaitForAndProcessIncomingLogEntries() - aborting");
          isDoneOrAborted = true;
          RcvFlush();
        }
        else if(isCompleted)
        {
          LogMsg(LogPrioInfo, "SyncManager::WaitForAndProcessIncomingLogEntries() - completed");
          
          isDoneOrAborted = true;
        }
        
      }
      else
      {
        LogMsg(LogPrioWarning, "SyncManager::WaitForAndProcessIncomingLogEntries() - received other message than LogEntry while waiting. ");
        incomingMsg.Print();
      }
    }
    else
    {
      auto elapsed = std::chrono::system_clock::now() - startOfWait;
      const bool isWaitOver = (elapsed > TimeToWaitInMsDuration);
      if(!isWaitOver)
      {
        OSATaskSleep(10);
      }
      else
      {
        LogMsg(LogPrioWarning, "SyncManager::WaitForAndProcessIncomingLogEntries() - Aborting (timedout) before whole log was received.");
        isDoneOrAborted = true;
      }
    }
    
  }while(!isDoneOrAborted);
  
}

void SyncManager::SendRequestForLog(std::string_view ipAddr)
{
  SyncMsg syncReq{SyncMsg::MsgType::RequestLog,m_gmm.GetMyId()};
  if(!Misc::SendToIp(syncReq, ipAddr, m_senders))
  {
    LogMsg(LogPrioError, "SyncManager::SendRequestForLog() - Failed to send LogRequest to %s",ipAddr);
  }

}

void SyncManager::HandleSyncReqToUpdateThisLog()
{
  if(m_isSyncTriggered)
  {
    m_isSyncTriggered = false;
    
  }
}

OSAStatusCode SyncManager::SyncTaskMethod()
{
  m_isRunning = true;
  do
  {
    HandleIncomingSyncRequestFromOthers();
    //Check if incoming sync request. That is request to send this instance of the LR to another node
      //Handle those
    //Check if this node has been requested to sync its log 
      //Handle that request
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  return OSA_OK;
}

void SyncManager::SendEntriesToRequester(std::string_view requesterIp, std::vector<std::shared_ptr<SyncMsg>> entriesToSend)
{
  for(auto &pEntryToSend : entriesToSend)
  {
    if(pEntryToSend)
    {
      if(!Misc::SendToIp(*pEntryToSend, requesterIp, m_senders))
      {
        LogMsg(LogPrioError, "SyncManager::SendEntriesToRequester() - failed to send to ip %s",requesterIp.data());
        break;
      }
    }
    else
    {
      LogMsg(LogPrioError, "SyncManager::SendEntriesToRequester() - Invalid pointer found in entries to send - aborting");
      break;
    }
    OSATaskSleep(10); //TODO: Just to decrease the probability of congestion. The prefered solution would be to use a TCP connection for the above sending...
  }
}

void SyncManager::RcvFlush()
{
  SyncMsg syncMsgToBeDiscarded{};
  while(RcvMsg(*m_pReceiver,syncMsgToBeDiscarded));
}

bool SyncManager::RcvMsg(IReceiver &rcv, SyncMsg &msg)
{
  return rcv.Rcv(msg);
}

OSAStatusCode SyncManager::ClassTaskMethod(void * const pInstance)
{
  return ((SyncManager*)(pInstance))->SyncTaskMethod();
}
