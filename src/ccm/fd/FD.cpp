#include "FD.h"    
#include "Logger.h"
#include "HeartbeatCCM.h"
#include "NwAid.h"
#include "../misc/Misc.h"
#include <errnoLib.h>


std::unique_ptr<FD> FD::CreateFD(GMM & gmm)
{
  static const int FAILURE_DETECTION_UDP_PORT = 4444;
  std::vector<std::unique_ptr<ISender>> senders;
  
  bool isAllCreationOk = Misc::CreateISendersFromMembersExcludingMySelf(FAILURE_DETECTION_UDP_PORT,gmm,senders);
  
  if(!isAllCreationOk)
  {
    LogMsg(LogPrioCritical, "ERROR FD::CreateFD failed to create sender(s). Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  else
  {
    std::unique_ptr<IReceiver> pRcv = NwAid::CreateUniCastReceiver(FAILURE_DETECTION_UDP_PORT);
    if(!pRcv)
    {
      LogMsg(LogPrioCritical, "ERROR FD::CreateFD failed to create receiver. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
      isAllCreationOk = false;
    }
    
    if(isAllCreationOk)
    {
      std::unique_ptr<FD> pFd = std::make_unique<FD>(gmm,senders,pRcv);
      if(!pFd)
      {
        LogMsg(LogPrioCritical, "ERROR FD::CreateFD failed to create FD.");
      }
      else
      {
        LogMsg(LogPrioInfo, "FD::CreateFD Successfully created a FD instance.");
        return pFd;
      }
      
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR FD::CreateFD failed to create sender.");
    }
  } 
  return nullptr;
}


FD::FD(GMM & gmm, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver) : m_gmm(gmm), //Reference to the GMM class, could be changed to a interface to make the dependency injection better. 
m_senders(),m_pReceiver(std::move(pReceiver))
{
  for(auto & pSender : senders)
  {
    m_senders.push_back(std::move(pSender));
  }
}

FD::~FD()
{
  
}

OSAStatusCode FD::Start()
{
  static const int TaskPrio = 50;   
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
  HeartbeatCCM outgoingHeartbeat;
  Populate(outgoingHeartbeat);
  Send(outgoingHeartbeat);
}

void FD::Populate(HeartbeatCCM &heartbeat)
{ 
  const int myId = m_gmm.GetMyId();
  const int leaderId = m_gmm.GetLeaderId();
  std::bitset<MAX_MEMBERS> myConnectionPerception;
  std::string myIp;
  
  
  m_gmm.ForMyMember([&myConnectionPerception,&myIp] (int id, Member& myself)
  {
    myConnectionPerception = myself.GetConnections();
    myIp = myself.GetIP();
  });
  
  heartbeat.SetConnectionPerception(myConnectionPerception);
  heartbeat.SetSenderId(myId);
  heartbeat.SetOutbound(true);
  heartbeat.SetSrcIp(myIp);
  heartbeat.SetLeaderId(leaderId);
  heartbeat.SetViewNumber(m_gmm.GetMyViewNumber());
  heartbeat.SetOpNumber(m_gmm.GetMyOpNumber());
  heartbeat.SetOpNumberCommitted(m_gmm.GetMyCommittedOpNumber());
}

void FD::Send(HeartbeatCCM &heartbeat)
{
  for(auto &pSender : m_senders)
  {
    if(!pSender->Send(heartbeat))
    {
      LogMsg(LogPrioInfo, "FD::Send - failed to send heartbeat");
    }
  }
}

void FD::HandleIncommingHeartbeat()
{
  HeartbeatCCM incomingHeartbeat;
  while(m_pReceiver->Rcv(incomingHeartbeat))
  {
    UpdateMemberWithReceivedHeartbeatData(incomingHeartbeat);
    UpdateMySelfWithReceivedHeartbeatData(incomingHeartbeat);
  };
  
}


void FD::UpdateMySelfWithReceivedHeartbeatData(const HeartbeatCCM &rcvdHeartbeat)
{
  const int hbSenderId = rcvdHeartbeat.GetSenderId();
  
  m_gmm.ForMyMember([this,&rcvdHeartbeat](int id, Member & myself)
   {
     myself.AddConnection(rcvdHeartbeat.GetSenderId());
     if(m_gmm.GetLeaderId() == rcvdHeartbeat.GetSenderId())
     {
       if(myself.GetViewNumber() < rcvdHeartbeat.GetViewNumber())
       {
         LogMsg(LogPrioInfo, "FD::UpdateMySelfWithReceivedHeartbeatData - updating view number %u %u",
             myself.GetViewNumber(), rcvdHeartbeat.GetViewNumber());
         myself.SetViewNumber(rcvdHeartbeat.GetViewNumber());
       }
     }
   });
}

void FD::UpdateMemberWithReceivedHeartbeatData(const HeartbeatCCM &rcvdHeartbeat)
{
  const int hbSenderId = rcvdHeartbeat.GetSenderId();
  
  m_gmm.ForIdMember(hbSenderId, [&rcvdHeartbeat](int id, Member & senderMember)
  {
    senderMember.SetConnectionPerception(rcvdHeartbeat.GetConnectionPerception());
    senderMember.UpdateHeartbeat(rcvdHeartbeat.GetLeaderId());
    senderMember.SetViewNumber(rcvdHeartbeat.GetViewNumber());
    senderMember.SetOperationNumber(rcvdHeartbeat.GetOpNumber());
    senderMember.SetCommittedOperationNumber(rcvdHeartbeat.GetOpNumberCommitted());
  });
}

void FD::CheckForFailingMembersAndUpdate()
{
  static const std::chrono::milliseconds HbTimeoutDuration(m_periodInMs * 3);
  m_gmm.UpdateConnectionStatusForMySelf(HbTimeoutDuration);
}

OSAStatusCode FD::FailureDetectionTaskMethod()
{
  m_isRunning = true;
  do
  {
    PopulateAndSendHeartbeat();
    HandleIncommingHeartbeat();
    CheckForFailingMembersAndUpdate();
    
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  return OSA_OK;
}

OSAStatusCode FD::ClassTaskMethod(void * const pInstance)
{
  return ((FD*)(pInstance))->FailureDetectionTaskMethod();
}
