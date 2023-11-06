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
  
  std::unique_ptr<PSRM> pPsrm = std::make_unique<PSRM>(pAir,pIccm);
  if(!pPsrm)
  {
    LogMsg(LogPrioCritical, "ERROR: PSRM::CreatePSRM failed to create PSRM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  return pPsrm;
}

PSRM::PSRM(std::unique_ptr<AIR>& pAir, std::shared_ptr<ICCM>& pIccm) : m_pAir{std::move(pAir)},m_pIccm{pIccm}
{
  
}

OSAStatusCode PSRM::Start()
{
  static const int TaskPrio = 30;   
  static const std::string TaskName("tPsrm");
  
  if(!m_pAir->RegisterWithCCM())
  {
    LogMsg(LogPrioError, "ERROR PSRM::Start Failed to register with CCM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
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
  LogMsg(LogPrioInfo, "--- <PSRM> ---");
}
