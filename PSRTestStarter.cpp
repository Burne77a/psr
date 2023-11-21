#include "PSRTestStarter.h"
#include "src/ccm/CCM.h"
#include "src/ccm/ICCM.h"
#include "src/psrm/PSRM.h"
#include "src/arf/ARF.h"
#include "src/apps/TestAppManager.h"

#include "Logger.h"
#include "TestClient.h"
#include <errnoLib.h>
#include <memory>

static std::shared_ptr<CCM> g_pCcm;

static std::shared_ptr<ICCM> g_pCsaIf;

static std::unique_ptr<TestClient> g_pTstClient;

static std::unique_ptr<PSRM> g_pPsrm;

static std::unique_ptr<ARF> g_pArf;

static std::unique_ptr<TestAppManager> g_pTstAppMgr;

static bool g_isRunning = false;

static const std::string g_backupIpAddr{"192.168.43.106"};


static const int g_BackupNodeId = 6;
static const int g_NumberOfBackupStorages = 4;

static int g_thisNodeId = 0;

static bool g_IsToRegisterStorageOnlyOnBackup{false};

static bool g_isStorageRegistred = false;
static bool g_isAppStarted = false;
static const unsigned int g_IterationsToWaitBeforeReg = 6U;
static const unsigned int g_MaxNumberOfNodes = 10U;

static void RegisterStorageForThisNode();
static void StartTestAppsForThisNode();

static unsigned int g_periodToUseForTestApps = 40U; //in ms
static unsigned int g_bytesToSyncEachPeriod = 1024U; 


void SetOneBackupStorage()
{
  g_IsToRegisterStorageOnlyOnBackup = true;
}

void SetPeriodAndBytes(const unsigned int period, const unsigned int bytes)
{
  g_periodToUseForTestApps = period;
  g_bytesToSyncEachPeriod = bytes;
}


OSAStatusCode StartPSRTest(const int id)
{
  LogMsgInit();
  
  g_thisNodeId = id;
  
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
  
  g_pArf = ARF::CreateARF(*g_pPsrm,g_BackupNodeId);

  if(!g_pArf)
  {
    LogMsg(LogPrioCritical, "ERROR: StartPSRTest CreateARF failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;
  }
  
  
  g_isRunning = true;
  
  g_pTstClient = std::make_unique<TestClient>(*g_pCsaIf);
  g_pTstClient->RegisterForUpcall();
  
  bool isLeaderAvilableLatched = false;
  
  do
  {
    if(isLeaderAvilableLatched != g_pCsaIf->IsThereALeader())
    {
      if(g_pCsaIf->IsThereALeader())
      {
        LogMsg(LogPrioInfo, "Leader became available");
        isLeaderAvilableLatched = true;
        
      }
      else
      {
        LogMsg(LogPrioInfo, "Leader lost");
        isLeaderAvilableLatched = false;
      }
      
    }
    RegisterStorageForThisNode();
    StartTestAppsForThisNode();
    OSATaskSleep(1000);
  }while(g_isRunning);

  return OSA_OK;
}

void RegisterStorageForThisNode()
{
  static const unsigned int IterationsToWaitBeforeReg = g_thisNodeId * g_IterationsToWaitBeforeReg;
  static unsigned int iterationCnt = 0;

  if(g_pCsaIf->IsThereALeader())
  {
    if(!g_isStorageRegistred)
    {
      if(iterationCnt >= IterationsToWaitBeforeReg)
      {
        if(!g_IsToRegisterStorageOnlyOnBackup || ((g_IsToRegisterStorageOnlyOnBackup) && (g_thisNodeId == g_BackupNodeId)))
        {
          AddStorage(10 + g_thisNodeId);
        }
        g_isStorageRegistred = true;
      }
      iterationCnt++;
    }
  }
}

void StartTestAppsForThisNode()
{
  static const unsigned int IterationsToWaitBeforeReg = (g_thisNodeId + g_MaxNumberOfNodes) * g_IterationsToWaitBeforeReg;
  static unsigned int iterationCnt = 0;

  if(g_pCsaIf->IsThereALeader() && g_isStorageRegistred)
  {
    if(!g_isAppStarted)
    {
      if(iterationCnt >= IterationsToWaitBeforeReg)
      {
        StartTestApps();
        g_isAppStarted = true;
      }
      iterationCnt++;
    }
  }
}

void Print()
{
  g_pCcm->Print();
  g_pPsrm->Print();
  g_pArf->Print();
  if(g_pTstAppMgr)
  {
    g_pTstAppMgr->Print();
  }
  else
  {
    LogMsg(LogPrioInfo, "Applications not created yet");
  }
}

void StartTestApps()
{
  g_pTstAppMgr = TestAppManager::CreateTestAppManager(g_thisNodeId,g_BackupNodeId, *g_pArf,g_backupIpAddr,g_periodToUseForTestApps,g_bytesToSyncEachPeriod);
  if(!g_pTstAppMgr)
  {
   LogMsg(LogPrioCritical, "ERROR: StartPSRTest CreateTestAppManager failed. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
   return;
  }
  
  g_pTstAppMgr->CreateApps();
  
  g_pTstAppMgr->StartApps();
}

void TriggerShutdown()
{
  g_pTstAppMgr->TriggerShutdownOfAppsOnThisNode();
}

void AddApp(unsigned int id)
{
  static const unsigned int bytes = 100U;
  static const unsigned int period = 200U;
  if(!g_pPsrm->RegisterApplication(id, g_thisNodeId, bytes, period))
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

void AddStorage(unsigned int id)
{
  static const unsigned int size = 100U;
  static const unsigned int bandwidth = 200U;
  static const std::string ipAddr{g_pCcm->GetIp(g_thisNodeId)};
  if(!g_pPsrm->RegisterStorage(id,g_thisNodeId, ipAddr, size, bandwidth))
  {
    LogMsg(LogPrioCritical, "ERROR: Failed to register storage");
  }
  else
  {
    LogMsg(LogPrioInfo, "Successfully registered storage %u on node %u",id,g_thisNodeId);
    if(g_IsToRegisterStorageOnlyOnBackup && g_BackupNodeId == g_thisNodeId)
    {
      for(int i = 0; i < g_NumberOfBackupStorages; i++)
      {
        const unsigned int additionalStorageId = id+1+i;
        LogMsg(LogPrioInfo, "This node is the sole storage, register one more storage"); 
        OSATaskSleep(1000);
        if(!g_pPsrm->RegisterStorage(additionalStorageId,g_thisNodeId, ipAddr, size, bandwidth))
        {
          LogMsg(LogPrioCritical, "ERROR: Failed to register storage");
        }
        else
        {
          LogMsg(LogPrioInfo, "Successfully registered additional storage %u on this backup node  %u",additionalStorageId ,g_thisNodeId);
        }
      }
    }
  }
}
void DelStorage(unsigned int id)
{
  if(!g_pPsrm->DeRegisterStorage(id))
  {
    LogMsg(LogPrioCritical, "ERROR: Failed to DE-register storage");
  }
  else
  {
    LogMsg(LogPrioInfo, "Successfully DE-registered storage");
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
