#include "PSRTestStarter.h"
#include "src/ccm/CCM.h"
#include "src/ccm/ICCM.h"
#include "src/psrm/PSRM.h"

#include "Logger.h"
#include "TestClient.h"
#include <errnoLib.h>
#include <memory>

static std::shared_ptr<CCM> g_pCcm;

static std::shared_ptr<ICCM> g_pCsaIf;

static std::unique_ptr<TestClient> g_pTstClient;

static std::unique_ptr<PSRM> g_pPsrm;

static bool g_isRunning = false;

OSAStatusCode StartPSRTest(const int id)
{
  LogMsgInit();
  
  g_pCcm = CCM::CreateAndInitForTest(id);
  if(!g_pCcm)
  {
     LogMsg(LogPrioCritical, "ERROR StartPSRTest CreateAndInitForTest failed to create CCM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
     return OSA_ERROR;
  }
  
  g_pCsaIf = g_pCcm;
  
  g_pPsrm = PSRM::CreatePSRM(g_pCsaIf);
  if(!g_pPsrm)
  {
    LogMsg(LogPrioCritical, "ERROR StartPSRTest CreateAndInitForTest failed to create PSRM. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  const OSAStatusCode ccmStartSts = g_pCcm->Start();
  if(ccmStartSts != OSA_OK)
  {
    LogMsg(LogPrioCritical, "ERROR: StartPSRTest CCM::Start failed %d. Errno: 0x%x (%s)",ccmStartSts,errnoGet(),strerror(errnoGet()));
    return ccmStartSts;
  }
  
  const OSAStatusCode psrmStartSts = g_pPsrm->Start();
  if(psrmStartSts != OSA_OK)
  {
    LogMsg(LogPrioCritical, "ERROR: StartPSRTest PSRM::Start failed %d. Errno: 0x%x (%s)",psrmStartSts,errnoGet(),strerror(errnoGet()));
    return psrmStartSts;
  }
  
  g_isRunning = true;
  
  g_pTstClient = std::make_unique<TestClient>(*g_pCsaIf);
  g_pTstClient->RegisterForUpcall();
  
  do
  {
    OSATaskSleep(1000);
  }while(g_isRunning);

  return OSA_OK;
}

void Print()
{
  g_pCcm->Print();
  g_pPsrm->Print();
}

void AddApp(unsigned int id)
{
  static const unsigned int bytes = 100U;
  static const unsigned int period = 200U;
  static const unsigned int primaryId = 1;
  if(!g_pPsrm->RegisterApplication(id, primaryId, bytes, period))
  {
    LogMsg(LogPrioCritical, "ERROR: Failed to register application");
  }
  else
  {
    LogMsg(LogPrioInfo, "Successfully registred application");
  }
}

void DelApp(unsigned int id)
{
  if(!g_pPsrm->DeRegisterApplication(id))
  {
    LogMsg(LogPrioCritical, "ERROR: Failed to DE-register application");
  }
  else
  {
    LogMsg(LogPrioInfo, "Successfully DE-registered application");
  }
}

void TriggerClientReq()
{
  g_pTstClient->IssueRequest();
}

void ForceUpcalls()
{
  g_pCcm->ForceUpcalls();
}
