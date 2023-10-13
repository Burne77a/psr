#include "src/ccm/CCM.h"
#include "Logger.h"
#include <errnoLib.h>
#include <memory>

static std::unique_ptr<CCM> g_pGmm;
static bool g_isRunning = false;

OSAStatusCode StartPSRTest(const int id)
{
  LogMsgInit();
  
  g_pGmm = CCM::CreateAndInitForTest(id);
  if(!g_pGmm)
  {
    LogMsg(LogPrioCritical, "ERROR StartPSRTest CreateAndInitForTest failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  g_isRunning = true;
  
  do
  {
    OSATaskSleep(1000);
  }while(g_isRunning);
  
  
  return OSA_OK;
}
