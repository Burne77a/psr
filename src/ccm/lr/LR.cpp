//Senders to all other members, that the leader uses to send request
//One receiver to receive log entries from the leader. 
//Client requests are handled by the CSA layer. one, when acting as leader, to receive client requests. 
#include "LR.h"
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
    pLr = std::make_unique<LR>(gmm,senders,pRcv);
    if(!pLr)
    {
      LogMsg(LogPrioCritical, "ERROR LR::CreateLR failed to create LR.");
    }
    else
    {
      LogMsg(LogPrioInfo, "LR::CreateLR Successfully created a LR instance.");
    }
  }
  return pLr;
}

LR::LR(GMM &gmm, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver) : m_gmm{gmm},m_pReceiver{std::move(pReceiver)}
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

void LR::RcvFlush()
{
  LogReplicationMsg lrMsgToBeDiscarded{LogReplicationMsg::MsgType::PrepareOK};
  while(RcvMsg(*m_pReceiver,lrMsgToBeDiscarded));
}

bool LR::RcvMsg(IReceiver &rcv, LogReplicationMsg &msg)
{
  return rcv.Rcv(msg);
}

void LR::Print() const
{
  LogMsg(LogPrioInfo, "--- LR ---");
  
  LogMsg(LogPrioInfo, "--- --- ---");
}


