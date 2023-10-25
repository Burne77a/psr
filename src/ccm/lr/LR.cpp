//Senders to all other members, that the leader uses to send request
//One receiver to receive log entries from the leader. 
//Client requests are handled by the CSA layer. one, when acting as leader, to receive client requests. 
#include "LR.h"
#include "ReplicatedLog.h"
#include "Logger.h"
#include "NwAid.h"
#include "../misc/Misc.h"
#include <errnoLib.h>
std::unique_ptr<LR> LR::CreateLR(GMM & gmm)
{
  static const int LOG_REPLICATION_UDP_PORT = 6666;
  std::vector<std::unique_ptr<ISender>> senders;
  bool isAllCreationOk = Misc::CreateISendersFromMembers(LOG_REPLICATION_UDP_PORT,gmm,senders);
  
  
  std::unique_ptr<IReceiver> pRcv = nullptr;
  std::unique_ptr<LR> pLr = nullptr;
  std::unique_ptr<ReplicatedLog> pReplicatedLog = nullptr;
    
  if(!isAllCreationOk)
  {
    LogMsg(LogPrioCritical, "ERROR LR::CreateLR failed to create sender(s). Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  else
  {
    pRcv = NwAid::CreateUniCastReceiver(LOG_REPLICATION_UDP_PORT);
    if(!pRcv)
    {
      LogMsg(LogPrioCritical, "ERROR LR::CreateLR failed to create receiver. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
      isAllCreationOk = false;
    }
  }
  
  if(isAllCreationOk)
  {
    pReplicatedLog = std::make_unique<ReplicatedLog>();
    
    if(pReplicatedLog)
    {
      pLr = std::make_unique<LR>(gmm,senders,pRcv,pReplicatedLog);
      if(!pLr)
      {
        LogMsg(LogPrioCritical, "ERROR LR::CreateLR failed to create LR.");
      }
      else
      {
        LogMsg(LogPrioInfo, "LR::CreateLR Successfully created a LR instance.");
      }
    }
  }
  return pLr;
}

LR::LR(GMM &gmm, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver,std::unique_ptr<ReplicatedLog> &pRepLog) : 
    m_gmm{gmm},m_pReceiver{std::move(pReceiver)}, m_pRepLog{std::move(pRepLog)}
{
  for(auto & pSender : senders)
  {
    m_senders.push_back(std::move(pSender));
  }
}

LR::~LR()
{
  
}

void LR::HandleActivityAsFollower()
{
  
}

void LR::HandleActivityAsLeader()
{
  
}

void LR::BecameLeaderActivity()
{
  LogMsg(LogPrioInfo, "LR::BecameLeaderActivity()");
  //Flush the incoming messages, since the former leader is gone, hence no need to process what is in the queue. 
  RcvFlush();
}

void LR::NoLongerLeaderActivity()
{
  LogMsg(LogPrioInfo, "LR::NoLongerLeaderActivity()");
}

void LR::PerformUpcalls()
{
  m_pRepLog->PerformUpcalls();
}

void LR::HandlePrepare(const LogReplicationMsg &lrMsg)
{
  const unsigned int msgViewNo = lrMsg.GetViewNumber();
  const unsigned int msgOpNo = lrMsg.GetOpNumber();
  const unsigned int myViewNo = m_gmm.GetMyViewNumber();
  const bool isMsgViewHigherOrEq = (myViewNo <= msgViewNo);
  if(isMsgViewHigherOrEq)
  {
    if(m_pRepLog->ArePreviousEntriesInLog(msgOpNo, msgViewNo))
    {
       if(m_pRepLog->AddEntryToLogIfNotAlreadyIn(lrMsg))
       {
         SendPrepareOK(lrMsg);
       }
       else
       {
         LogMsg(LogPrioCritical, "ERROR LR::HandlePrepare() Failed to add entry to log ");
         lrMsg.Print();
       }
    }
    else
    {
#warning trigger sync 
    }
  }
  else
  {
    LogMsg(LogPrioInfo, "LR::HandlePrepare() received Prepare with too low view number - ignoring %u %u ",myViewNo,msgViewNo);
    lrMsg.Print();
  }
  //Check if this follower is uptodate
  //If it is, add to log, and send prepare OK
  //If not, trigger sync.
  //What to do with the current request?
}

void LR::HandleCommit(const LogReplicationMsg &lrMsg)
{
  const unsigned int msgViewNo = lrMsg.GetViewNumber();
  const unsigned int msgOpNo = lrMsg.GetOpNumber();
  const unsigned int myViewNo = m_gmm.GetMyViewNumber();
  const bool isMsgViewHigherOrEq = (myViewNo <= msgViewNo);
  if(isMsgViewHigherOrEq)
  {
    if(m_pRepLog->IsEntryAlreadyExisting(lrMsg))
    {
      if(!m_pRepLog->CommitEntryIfPresent(lrMsg))
      {
        LogMsg(LogPrioCritical, "LR::HandleCommit() failed to commit entry for commit message");
        lrMsg.Print();
      }
    }
    else
    {
      LogMsg(LogPrioInfo, "LR::HandleCommit() received Commit for missing entry, triggering sync ");
      #warning trigger sync 
    }
  }
  else
  {
    LogMsg(LogPrioError, "LR::HandleCommit() received Commit with too low view number - ignoring %u %u ",myViewNo,msgViewNo);
    lrMsg.Print();
  }
}


void LR::HandleMsgAsFollower()
{
  LogReplicationMsg lrMsg{LogReplicationMsg::MsgType::PrepareOK};
  while(RcvMsg(*m_pReceiver,lrMsg))
  {
    if(lrMsg.IsPrepareMsg())
    {
      HandlePrepare(lrMsg);
    }
    else if(lrMsg.IsPrepareOKMsg())
    {
      LogMsg(LogPrioInfo, "LR::HandlePrepare() received PrepareOK as follower from %u (%u:%u) ",lrMsg.GetSrcId(),lrMsg.GetViewNumber(),lrMsg.GetOpNumber());
    }
    else if(lrMsg.IsCommitMsg())
    {
      HandleCommit(lrMsg);
    }
    else
    {
      LogMsg(LogPrioCritical, "LR::HandleMsgAsFollower() - Unknown message type.");
      lrMsg.Print();
    }
  };
}

void LR::RcvFlush()
{
  LogReplicationMsg lrMsgToBeDiscarded{LogReplicationMsg::MsgType::PrepareOK};
  while(RcvMsg(*m_pReceiver,lrMsgToBeDiscarded));
}

bool LR::RcvMsg(IReceiver &rcv, LogReplicationMsg &msg)
{
  return rcv.Rcv(msg);
}

void LR::SendPrepareOK(const LogReplicationMsg & prepareMsg)
{
  LogReplicationMsg prepareOkMsg{LogReplicationMsg::MsgType::PrepareOK,prepareMsg.GetOpNumber(),prepareMsg.GetViewNumber(),static_cast<unsigned int>(m_gmm.GetMyId()),prepareMsg.GetSrcId()};
  if(!SendToLeader(prepareOkMsg))
  {
    LogMsg(LogPrioError, "ERROR: LR::SendPrepareOK() - failed to send PrepareOK.");
    prepareMsg.Print();
  }

}

bool LR::SendToLeader(const LogReplicationMsg & msgToSend)
{
  const std::string leaderIpAddr{m_gmm.GetLeaderIp()};
  bool isSuccessfullySent = false;
  bool isLeaderFound = false;
  for(auto & pSender : m_senders)
  {
    if(pSender)
    {
      if(pSender->GetIpAddr().compare(leaderIpAddr) == 0)
      {
        isLeaderFound = true;
        isSuccessfullySent = pSender->Send(msgToSend);
      }
    }
  }
  if(!isLeaderFound)
  {
    LogMsg(LogPrioError, "ERROR: LR::SendToLeader() - failed. No leader found. Leader IP %s ",leaderIpAddr.c_str());
  }
  else
  {
    if(!isSuccessfullySent)
    {
      LogMsg(LogPrioError, "ERROR: LR::SendToLeader() - failed to send. Leader IP %s ",leaderIpAddr.c_str());
    }
  }
  return isSuccessfullySent;
}

void LR::Print() const
{
  LogMsg(LogPrioInfo, "--- LR ---");
  m_pRepLog->Print();
  
  LogMsg(LogPrioInfo, "--- --- ---");
}


