#include "CCM.h"
#include "Logger.h"
#include "NwAid.h"
#include <errnoLib.h>

static constexpr std::string_view IP_ADDRESS_OF_LEADER{"192.168.213.10"};

std::shared_ptr<CCM> CCM::CreateAndInitForTest(const int myId)
{
 
  std::unique_ptr<GMM> pGmm = std::make_unique<GMM>(myId);
  if(!pGmm)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::CreateAndInitForTest failed to create GMM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  pGmm->AddMember(1, "Instance 1", "192.168.213.101");
  pGmm->AddMember(2, "Instance 2", "192.168.213.102");
  pGmm->AddMember(3, "Instance 3", "192.168.213.103");
  
  std::unique_ptr<FD> pFd = FD::CreateFD(*pGmm);
  if(!pFd)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::CreateAndInitForTest failed to create FD. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<LE> pLe = LE::CreateLE(*pGmm);
  if(!pLe)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::CreateAndInitForTest failed to create LE. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<LR> pLr = LR::CreateLR(*pGmm);
  if(!pLr)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::CreateAndInitForTest failed to create LR. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::shared_ptr<CCM> pCmm = std::make_shared<CCM>(pGmm,pFd,pLe,pLr);
  
  if(!pCmm)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::CreateAndInitForTest failed to create CCM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  return pCmm;
}

CCM::CCM(std::unique_ptr<GMM> &pGmm, std::unique_ptr<FD> &pFd, std::unique_ptr<LE> &pLe,std::unique_ptr<LR> &pLr) : 
    m_pGmm{std::move(pGmm)},m_pFd{std::move(pFd)},m_pLe{std::move(pLe)},m_pLr{std::move(pLr)}
{
 
 
}

CCM::~CCM()
{
  
}

void CCM::Stop()
{
  LogMsg(LogPrioInfo, "LE::Stop() - Commanding LE task to end. ");
  m_isRunning = false;
}

OSAStatusCode CCM::Start()
{
  const OSAStatusCode fdStartSts = m_pFd->Start();
  m_leaderCb = shared_from_this(); //TODO: This is stupid, but will do for now. 
  m_pLe->SetLeaderCallback(m_leaderCb);
  if(fdStartSts != OSA_OK)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::Start failed to start FD task. %d Errno: 0x%x (%s)",fdStartSts,errnoGet(),strerror(errnoGet()));
    return fdStartSts;
  }
  
  const OSAStatusCode ccmStartSts = StartCCMTask();
  if(ccmStartSts != OSA_OK)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::Start failed to start CCM task. %d Errno: 0x%x (%s)",ccmStartSts,errnoGet(),strerror(errnoGet()));
    m_pFd->Stop();
    return ccmStartSts;
  }
  
  LogMsg(LogPrioInfo, "CCM::Start successful");
  return OSA_OK;
}

std::string_view CCM::GetLeaderIp()
{
  return IP_ADDRESS_OF_LEADER;
}

void CCM::EnteredLeaderRole() 
{
  LogMsg(LogPrioInfo, "CCM::EnteredLeaderRole() - Assigning leader IP address %s to this node.",IP_ADDRESS_OF_LEADER);
  //TODO: Hardcoded interfaces for the floating leader IP address is most likely not the best idea in the long run, but it works for now. 
  NwAid::AddIpOnNwIf(NwAid::EthIfNo::IfTwo, IP_ADDRESS_OF_LEADER);
  NwAid::ArpFlush();
  //TODO: Ensure that the leader is synchronized and up to date, here or at other suitable location. 
  m_pLr->BecameLeaderActivity();
}

void CCM::LeftLeaderRole()
{
  LogMsg(LogPrioInfo, "CCM::LeftLeaderRole() - Removing leader IP address %s from this node.",IP_ADDRESS_OF_LEADER);
  //TODO: Hardcoded interfaces for the floating leader IP address is most likely not the best idea in the long run, but it works for now. 
  NwAid::RemoveIpOnNwIf(NwAid::EthIfNo::IfTwo, IP_ADDRESS_OF_LEADER);
  NwAid::ArpFlush();
  
  m_pLr->NoLongerLeaderActivity();
}

OSAStatusCode CCM::StartCCMTask()
{
  static const int TaskPrio = 30;   
  static const std::string TaskName("tCcm");
  
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)CCM::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
    LogMsg(LogPrioError, "ERROR CCM::Start Failed to spawn CCM task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  return OSA_OK;
}

void CCM::MakeUpcalls()
{
  m_pLr->PerformUpcalls();
}

OSAStatusCode CCM::InstanceTaskMethod()
{
  m_isRunning = true;
  do
  {
    m_pLe->HandleActivity();
    if(m_pLe->GetCurrentStateValue() == StateBaseLE::StateValue::Leader)
    {
      m_pLr->HandleActivityAsLeader();
    }
    else if(m_pLe->GetCurrentStateValue() == StateBaseLE::StateValue::Follower)
    {
      m_pLr->HandleActivityAsFollower();
    }
    
    MakeUpcalls();
    //Read from client serving IP address.
    //Pass request to LR
    //
    
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  return OSA_OK;
}

OSAStatusCode CCM::ClassTaskMethod(void * const pInstance)
{
  return ((CCM*)(pInstance))->InstanceTaskMethod();
}

void CCM::Print() const
{
  m_pGmm->Print();
  m_pLe->Print();
  m_pLr->Print();
}



