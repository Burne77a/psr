#include "PSRTestStarter.h"
#include "src/ccm/CCM.h"
#include "src/ccm/ICCM.h"

#include "Logger.h"
#include "TestClient.h"
#include <errnoLib.h>
#include <memory>

static std::shared_ptr<CCM> g_pCcm;

static std::shared_ptr<ICCM> g_pCsaIf;

static bool g_isRunning = false;

OSAStatusCode StartPSRTest(const int id)
{
  LogMsgInit();
  
  g_pCcm = CCM::CreateAndInitForTest(id);
   if(!g_pCcm)
   {
     LogMsg(LogPrioCritical, "ERROR StartPSRTest CreateAndInitForTest failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
     return OSA_ERROR;
   }
   
   g_pCcm->Start();
   
   g_pCsaIf = g_pCcm;
   
   g_isRunning = true;
   
   do
   {
     OSATaskSleep(1000);
   }while(g_isRunning);
  
  return OSA_OK;
}

void Print()
{
  g_pCcm->Print();
}

void TriggerClientReq()
{
  
}
