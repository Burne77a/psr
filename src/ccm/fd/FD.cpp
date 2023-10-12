#include "FD.h"    
#include "Logger.h"
#include "HeartbeatCCM.h"
#include <errnoLib.h>
#include <vector>
FD::FD(GMM & gmm,ISender & sender, IReceiver receiver) : m_gmm(gmm), //Reference to the GMM class, could be changed to a interface to make the dependency injection better. 
m_sender(sender),m_receiver(receiver)
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
  std::vector<HeartbeatCCM> outgoingHeartbeats;
  Populate(outgoingHeartbeats);
  Send();
}

void FD::Populate(std::vector<HeartbeatCCM> &heartbeats)
{
  //vector with heartbeats, populated by the lambda function below
  
  const int myId = m_gmm.GetMyId();
  std::bitset<MAX_MEMBERS> myConnectionPerception;
  std::string myIp;
  
  m_gmm.ForMyMember([&myConnectionPerception] (int id, Member& myself)
  {
    myConnectionPerception = myself.GetConnections();
    myIp = myself.GetIP();
  });
  
  m_gmm.ForEachMember([&heartbeats,myId,&myConnectionPerception,&myIp](int id, Member& member) 
  {
    if(myId != member.GetID())
    {
      HeartbeatCCM hb;
    
      hb.SetOutbound(true);
      hb.SetDstIp(member.GetIP());
      hb.SetSrcIp(myIp.GetIP());
      
      hb.SetConnectionPerception(myConnectionPerception);
      hb.SetSenderId(myId);
     
   
      heartbeats.push_back(hb);
    }
  });
}

void FD::Send(std::vector<HeartbeatCCM> &heartbeats)
{
  //If this would be multicast, only one send would be needed.
  for(auto hb : heartbeats)
  {
    m_sender.Send(hb);
  }
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
