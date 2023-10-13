#include "CCM.h"
#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<CCM> CCM::CreateAndInitForTest(const int myId)
{
 
  std::unique_ptr<GMM> pGmm = std::make_unique<GMM>(myId);
  if(!pGmm)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::CreateAndInitForTest failed to create GMM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  pGmm->AddMember(1, "Instance 1", "192.168.101.1");
  pGmm->AddMember(2, "Instance 2", "192.168.101.2");
  pGmm->AddMember(3, "Instance 3", "192.168.101.3");
  
  std::unique_ptr<FD> pFd = FD::CreateFD(*pGmm);
  if(!pFd)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::CreateAndInitForTest failed to create FD. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<CCM> pCmm = std::make_unique<CCM>(pGmm,pFd);
  
  if(!pCmm)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::CreateAndInitForTest failed to create CCM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  return pCmm;
}

CCM::CCM(std::unique_ptr<GMM> &pGmm, std::unique_ptr<FD> &pFd) : m_pGmm(std::move(pGmm)),m_pFd(std::move(pFd))
{
  
}

CCM::~CCM()
{
  
}

OSAStatusCode CCM::Start()
{
  const OSAStatusCode fdStartSts = m_pFd->Start();
  if(fdStartSts != OSA_OK)
  {
    LogMsg(LogPrioCritical, "ERROR CCM::Start failed to start FD task. %d Errno: 0x%x (%s)",fdStartSts,errnoGet(),strerror(errnoGet()));
    return fdStartSts;
  }
  LogMsg(LogPrioInfo, "CCM::Start successful");
  return OSA_OK;
}



