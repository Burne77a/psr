#include "StateRetriever.h"
#include "Logger.h"
#include "NwAid.h"
#include "TaskAbstraction.h"
#include <errnoLib.h>



std::unique_ptr<StateRetriever> StateRetriever::CreateStateRetriever(const unsigned int appId, const unsigned int port, std::string_view storageIp)
{
  std::unique_ptr<StateRetriever> pSr{nullptr};
  
  
  std::unique_ptr<IReceiver> pRcv = NwAid::CreateUniCastReceiver(port);
  if(!pRcv)
  {
    LogMsg(LogPrioCritical, "ERROR: StateRetriever::CreateStateRetriever failed to create IReceiver (port %d). Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  
  std::unique_ptr<ISender> pSnd = NwAid::CreateUniCastSender(storageIp,port);
  if(!pSnd)
  {
    LogMsg(LogPrioCritical, "ERROR: StateRetriever::CreateStateRetriever failed to create ISender (port %d). Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
    
  
  pSr = std::make_unique<StateRetriever>(appId,pSnd,pRcv);
  if(!pSr)
  {
    LogMsg(LogPrioCritical, "ERROR: StateRetriever::CreateStateRetriever failed to create StateRetriever. appId: %u  Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
  }
  
  return pSr;
}

StateRetriever::StateRetriever(const unsigned int appId,std::unique_ptr<ISender> &pSender, std::unique_ptr<IReceiver> &pReceiver) 
: m_appId{appId},m_pSender{std::move(pSender)},m_pReceiver{std::move(pReceiver)}
{
  
}


bool StateRetriever::GetLatestStateFromStorage(ISerializable & objToPopulate)
{
  StateDataMsg msgToReqLatestState{StateDataMsg::MsgType::Get};
  if(!m_pSender->Send(msgToReqLatestState))
  {
    LogMsg(LogPrioCritical, "ERROR: StateRetriever::GetLatestStateFromStorage failed to send request for state (appId %u). Errno: 0x%x (%s)",m_appId,errnoGet(),strerror(errnoGet()));
    return false;
  }
  
  StateDataMsg latestStateMsg;
  if(!WaitForReply(latestStateMsg))
  {
    LogMsg(LogPrioCritical, "ERROR: StateRetriever::GetLatestStateFromStorage did not get state reply appId %u",m_appId);
    return false;
  }
  
  if(!PopulateFromRcvdState(latestStateMsg,objToPopulate))
  {
    LogMsg(LogPrioCritical, "ERROR: StateRetriever::GetLatestStateFromStorage failed to populate app state with received data AppId %u",m_appId);
    return false;
  }
  
  return true;
}



bool StateRetriever::PopulateFromRcvdState(const StateDataMsg &msgRcvd, ISerializable & objToPopulate)
{
  bool isSuccessfullyPopulated = false;
  unsigned int payloadSize = 0U;
  const uint8_t * pPayloadBuffer = msgRcvd.GetPayloadBuffer(payloadSize);
  if((payloadSize > 0) && (pPayloadBuffer != nullptr))
  {
    unsigned int inBufferSize = 0U;
    const uint8_t * pInbuffer = objToPopulate.GetSerializableDataBuffer(inBufferSize);
    if((pInbuffer != nullptr) && (inBufferSize >= payloadSize))
    {
      std::memcpy((void*)pInbuffer,pPayloadBuffer,payloadSize);
      isSuccessfullyPopulated = true;
    }
    else
    {
      LogMsg(LogPrioCritical,"ERROR: StateRetriever::PopulateFromRcvdState invalid inbuffers 0x%x %u %u", pInbuffer,inBufferSize,payloadSize);
    }
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: StateRetriever::PopulateFromRcvdState invalid payload buffers 0x%x %u", pPayloadBuffer,payloadSize);
  }
  return isSuccessfullyPopulated;
}

bool StateRetriever::WaitForReply(StateDataMsg &msgRcvd)
{
  bool gotReply = false;
  bool hasTimedout = false;
  const unsigned int MaxIterations = 500;
  unsigned int iterationCnt = 0;
  do
  {
    if(m_pReceiver->Rcv(msgRcvd))
    {
      if(msgRcvd.IsFetchedMsg())
      {
        gotReply = true;
        LogMsg(LogPrioInfo,"StateRetriever::WaitForReply for appId %u got reply after %u iterations",m_appId,iterationCnt);
      }
      else
      {
        LogMsg(LogPrioCritical, "WARNING: StateRetriever::WaitForReply for appId %u got msg of unexpected type",m_appId);
        msgRcvd.Print();
      }
    }
    if(!gotReply)
    {
      iterationCnt++;
      if(iterationCnt < MaxIterations)
      {
        OSATaskSleep(2);
      }
      else
      {
        LogMsg(LogPrioCritical, "WARNING: StateRetriever::WaitForReply for appId %u timedout %u",m_appId,iterationCnt);
        hasTimedout = true;
      }
    }
  }while(!gotReply && !hasTimedout);
  
  return gotReply;
}

  
void StateRetriever::Print() const
{
  LogMsg(LogPrioInfo,"--- >StateRetriever< ---");
  LogMsg(LogPrioInfo,"AppId: %u ",m_appId);
  LogMsg(LogPrioInfo,"--- <StateRetriever> ---");
}



