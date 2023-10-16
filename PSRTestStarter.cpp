#include "PSRTestStarter.h"
#include "src/ccm/CCM.h"
#include "Logger.h"
#include <errnoLib.h>
#include <memory>

static std::unique_ptr<CCM> g_pCmm;
static bool g_isRunning = false;

OSAStatusCode StartPSRTest(const int id)
{
  LogMsgInit();
  
  g_pCmm = CCM::CreateAndInitForTest(id);
  if(!g_pCmm)
  {
    LogMsg(LogPrioCritical, "ERROR StartPSRTest CreateAndInitForTest failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  g_pCmm->Start();
  
  g_isRunning = true;
  
  do
  {
    OSATaskSleep(1000);
  }while(g_isRunning);
  
  
  return OSA_OK;
}

void Print()
{
  g_pCmm->Print();
}
