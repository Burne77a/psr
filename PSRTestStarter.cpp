#include "PSRTestStarter.h"
#include "src/ccm/CSA/CSA.h"

#include "Logger.h"
#include "TestClient.h"
#include <errnoLib.h>
#include <memory>

static std::unique_ptr<CSA> g_pCsa;

static std::unique_ptr<TestClient> g_pTc;

static bool g_isRunning = false;

OSAStatusCode StartPSRTest(const int id)
{
  LogMsgInit();
  
  g_pCsa = CSA::CreateCSA(id);
  if(!g_pCsa)
  {
    LogMsg(LogPrioCritical, "ERROR: StartPSRTest CSA::CreateCSA failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  const OSAStatusCode startSts = g_pCsa->Start();
  if(startSts != OSA_OK)
  {
    LogMsg(LogPrioCritical, "ERROR: StartPSRTest Start CSA (CMM) failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  g_isRunning = true;
  
  g_pTc = std::make_unique<TestClient>(*g_pCsa);
  if(!g_pTc)
  {
    LogMsg(LogPrioCritical, "ERROR: StartPSRTest Failed to create test client (TestClient). Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  do
  {
    OSATaskSleep(1000);
  }while(g_isRunning);
  
  
  return OSA_OK;
}

void Print()
{
  g_pCsa->Print();
}

void TriggerClientReq()
{
  
}
