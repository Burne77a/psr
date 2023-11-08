#include "PSRM.h"
#include "air/AIR.h"

#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<PSRM> PSRM::CreatePSRM(std::shared_ptr<ICCM>& pIccm)
{
  std::unique_ptr<AIR> pAir = AIR::CreateAIR(pIccm);
  if(!pAir)
  {
    LogMsg(LogPrioCritical, "ERROR: PSRM::CreatePSRM failed to create AIR. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<SR> pSr = SR::CreateSR(pIccm);
  if(!pSr)
  {
    LogMsg(LogPrioCritical, "ERROR: PSRM::CreatePSRM failed to create SR. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<SSPR> pSspr = SSPR::CreateSSPR(pIccm);
  if(!pSspr)
  {
    LogMsg(LogPrioCritical, "ERROR: PSRM::CreatePSRM failed to create SSPR. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<ASSP> pAssp = ASSP::CreateASSP(pIccm, *pAir, *pSr, *pSspr);
  if(!pAssp)
  {
    LogMsg(LogPrioCritical, "ERROR: PSRM::CreatePSRM failed to create ASSP. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<PSRM> pPsrm = std::make_unique<PSRM>(pAir,pSr,pSspr,pAssp,pIccm);
  if(!pPsrm)
  {
    LogMsg(LogPrioCritical, "ERROR: PSRM::CreatePSRM failed to create PSRM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  return pPsrm;
}

PSRM::PSRM(std::unique_ptr<AIR>& pAir,std::unique_ptr<SR>& pSr,std::unique_ptr<SSPR>& pSspr,std::unique_ptr<ASSP>& pAssp, std::shared_ptr<ICCM>& pIccm) : 
    m_pAir{std::move(pAir)},m_pSr{std::move(pSr)},m_pSspr{std::move(pSspr)},m_pAssp{std::move(pAssp)},m_pIccm{pIccm}
{
  
}

OSAStatusCode PSRM::Start()
{
  static const int TaskPrio = 70;   
  static const std::string TaskName("tPsrm");
  
  if(!m_pAir->RegisterWithCCM())
  {
    LogMsg(LogPrioError, "ERROR PSRM::Start Failed to register AIR with CCM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  if(!m_pSr->RegisterWithCCM())
  {
    LogMsg(LogPrioError, "ERROR PSRM::Start Failed to register SR with CCM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  if(!m_pSspr->RegisterWithCCM())
  {
    LogMsg(LogPrioError, "ERROR PSRM::Start Failed to register SSPR with CCM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
   
 
  const OSATaskId taskId = OSACreateTask(TaskName,TaskPrio,(OSATaskFunction)PSRM::ClassTaskMethod,(OSAInstancePtr)this);
  
  if(taskId == OSA_ERROR)
  {
   LogMsg(LogPrioError, "ERROR PSRM::Start Failed to spawn CCM task. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
   return OSA_ERROR;
  }
  return OSA_OK;
}

void PSRM::Stop()
{
  LogMsg(LogPrioInfo, "PSRM::Stop() - Commanding PSRM task to end. ");
  m_isRunning = false;
}

OSAStatusCode PSRM::InstanceTaskMethod()
{
  
  m_isRunning = true;
  do
  {
    m_pSspr->HandleActivity();
    m_pAssp->HandleActivity();
    
    OSATaskSleep(m_periodInMs);
  }while(m_isRunning);
  return OSA_OK;
}

OSAStatusCode PSRM::ClassTaskMethod(void * const pInstance)
{
  return ((PSRM*)(pInstance))->InstanceTaskMethod();
}

void PSRM::Print() const
{
  LogMsg(LogPrioInfo, "--- >PSRM<---");
  m_pAir->Print();
  m_pSr->Print();
  m_pSspr->Print();
  m_pAssp->Print();
  LogMsg(LogPrioInfo, "--- <PSRM> ---");
}
