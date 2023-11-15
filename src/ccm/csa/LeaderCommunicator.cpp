#include "LeaderCommunicator.h"
#include "NwAid.h"
#include "Logger.h"
#include "TaskAbstraction.h"
#include "../misc/Misc.h"
#include <errnoLib.h>
#include <chrono>

std::unique_ptr<LeaderCommunicator> LeaderCommunicator::CreateLeaderCommunicator(const std::string_view leaderIpAddress, const int leaderRcvPort,GMM & gmm, const int leaderSndPort)
{
  std::unique_ptr<IReceiver> pLeaderRcv = NwAid::CreateUniCastReceiver(leaderRcvPort);
  if(!pLeaderRcv)
  {
    LogMsg(LogPrioCritical, "ERROR: LeaderCommunicator::CreateLeaderCommunicator failed to create leader receiver. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<ISender> pLeaderSnd = NwAid::CreateUniCastSender(leaderIpAddress, leaderRcvPort);
  if(!pLeaderSnd)
  {
    LogMsg(LogPrioCritical, "ERROR: LeaderCommunicator::CreateLeaderCommunicator failed to create leader sender. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::vector<std::unique_ptr<ISender>> clientSenders; 
  if(!Misc::CreateISendersFromMembersIncludingMySelf(leaderSndPort, gmm, clientSenders))
  {
    LogMsg(LogPrioCritical, "ERROR: LeaderCommunicator::CreateLeaderCommunicator failed to create client senders. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<IReceiver> pClientRcv = NwAid::CreateUniCastReceiver(leaderSndPort);
  if(!pClientRcv)
  {
    LogMsg(LogPrioCritical, "ERROR: LeaderCommunicator::CreateLeaderCommunicator failed to create client receiver. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<LeaderCommunicator> pLc = std::make_unique<LeaderCommunicator>(pLeaderSnd,pLeaderRcv,clientSenders,pClientRcv);
  if(!pLc)
  {
    LogMsg(LogPrioCritical, "ERROR: LeaderCommunicator::CreateLeaderCommunicator failed to create LeaderCommunicator. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  return pLc;
}

LeaderCommunicator::LeaderCommunicator(std::unique_ptr<ISender>& pLeaderSnd, std::unique_ptr<IReceiver>&  pLeaderRcv, std::vector<std::unique_ptr<ISender>>& clientSenders, std::unique_ptr<IReceiver>& pClientRcv) : 
    m_pLeaderSnd{std::move(pLeaderSnd)}, m_pLeaderRcv{std::move(pLeaderRcv)}, m_pClientRcv{std::move(pClientRcv)}
{
  for(auto & pSender : clientSenders)
  {
    m_clientSenders.push_back(std::move(pSender));
  }
}

bool LeaderCommunicator::WaitForCommitAckToRequest(const ClientMessage &req,const unsigned int timeToWaitInMs)
{
  static const std::chrono::milliseconds TimeToWaitInMsDuration (timeToWaitInMs * 3);
  bool isCorrectReplyReceived = false;
  bool isWaitOver = false;
  auto startOfWait = std::chrono::system_clock::now();
  do
  {
    bool isReplyReceived = false;
    ClientMessage reply;
    isReplyReceived = ReceiveFromLeader(reply);
    if(isReplyReceived)
    {
      isCorrectReplyReceived = IsValidReply(req,reply);
    }
    else
    {
      auto elapsed = std::chrono::system_clock::now() - startOfWait;
      isWaitOver = (elapsed > TimeToWaitInMsDuration);
      if(!isWaitOver)
      {
        OSATaskSleep(200);
      }
    }
  }while(!isCorrectReplyReceived && !isWaitOver);
  if(!isCorrectReplyReceived)
  {
    LogMsg(LogPrioError, "ERROR: LeaderCommunicator::WaitForCommitAckToRequest no reply received for request");
    req.Print();
  }
  return isCorrectReplyReceived;
}

bool LeaderCommunicator::IsValidReply(const ClientMessage &req,const ClientMessage &reply)
{
  bool isValidReply = false;
  if(reply.GetReqId() == req.GetReqId())
  {
    if(reply.IsAck())
    {
      isValidReply = true;
    }
    else if(reply.IsNAck())
    {
      LogMsg(LogPrioError, "WARNING: LeaderCommunicator::IsValidReply Received reply NACK");
    }
    else
    {
      LogMsg(LogPrioError, "WARNING: LeaderCommunicator::IsValidReply Received reply request as reply");
    } 
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: LeaderCommunicator::IsValidReply Received reply to another request");
  }
  if(!isValidReply)
  {
    reply.Print();
  }
  return isValidReply;
}

bool LeaderCommunicator::SendToLeaderWithRetries(const ClientMessage &msg, const unsigned int retries, const unsigned int timeBetweenInMs)
{
  bool isSuccessfullySent = false;
  unsigned int retryCnt = 0U;
  do
  {
    isSuccessfullySent = SendToLeader(msg);
    if(!isSuccessfullySent)
    {
      retryCnt++;
      OSATaskSleep(timeBetweenInMs);
    }
    
  }while(!isSuccessfullySent && (retryCnt < retries));
  
  if(!isSuccessfullySent)
  {
    LogMsg(LogPrioError, "ERROR: LeaderCommunicator::SendToLeaderWithRetries failed to send message to leader after %u retries. Errno: 0x%x (%s)",retries,errnoGet(),strerror(errnoGet()));
  }
  
  return isSuccessfullySent;
}

void LeaderCommunicator::FlushMsgToLeader()
{
  ClientMessage msgToDiscard;
  while(ReceiveFromClient(msgToDiscard));
}


bool LeaderCommunicator::GetClientRequestsSentToLeader(ClientMessage & msg)
{
  bool isClientReqMsgReceived = false;
  bool isToTryOneMoreTime = false;
  do
  {
    isToTryOneMoreTime = false;
    const bool isMsgRcvd = ReceiveFromClient(msg);
    if(isMsgRcvd)
    {
      if(!msg.IsRequest())
      {
        LogMsg(LogPrioWarning, "ERROR: LeaderCommunicator::GetClientRequestsSentToLeader Received message as leader that is not a request - discarding and trying again");
        msg.Print();
        isToTryOneMoreTime = true;
      }
      else
      {
        isClientReqMsgReceived = true;
      }
    }
  }while(isToTryOneMoreTime);
  
  return isClientReqMsgReceived;
}

bool LeaderCommunicator::SendToClient(ClientMessage & msg,const GMM & gmm)
{
  ClientRequestId reqId = msg.GetReqId();
  if(!reqId.IsValid())
  {
    LogMsg(LogPrioError, "ERROR: CSA::SendReplyToClient failed - cannot send reply with invalid req id");
    return false;
  }
  const unsigned int clientId = reqId.GetId();
  const std::string clientIp{gmm.GetIp(clientId)};
  
  if(!Misc::SendToIp(msg, clientIp, m_clientSenders))
  {
    LogMsg(LogPrioError, "ERROR: CSA::SendReplyToClient failed");
    return false;
  }
  return true;
}



bool LeaderCommunicator::ReceiveFromClient(ClientMessage &msg)
{
  const bool isSuccessfullyRcvd = m_pLeaderRcv->Rcv(msg);
  return isSuccessfullyRcvd;  
}

bool LeaderCommunicator::ReceiveFromLeader(ClientMessage &rcvdMsg)
{
  const bool isSuccessfullyRcvd = m_pClientRcv->Rcv(rcvdMsg);
  return isSuccessfullyRcvd;
}

bool LeaderCommunicator::SendToLeader(const ClientMessage &msg)
{
  bool isSentSuccessfully = false;
  isSentSuccessfully = m_pLeaderSnd->Send(msg);
  if(!isSentSuccessfully)
  {
    LogMsg(LogPrioError, "ERROR: LeaderCommunicator::SendToLeader failed to send message to leader. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
  return isSentSuccessfully;
}
