#include "TestAppManager.h"
#include "tstApps/FirstSimpleTestApp.h"
#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<TestAppManager> TestAppManager::CreateTestAppManager(const int nodeId,const int backupNodeId,ARF &arf, std::string_view backupNodeIpAddr,const unsigned int period, const unsigned int bytes)
{
  std::unique_ptr<TestAppManager> pTstAppMgr = std::make_unique<TestAppManager>(nodeId,backupNodeId,arf,backupNodeIpAddr,period, bytes);
  if(!pTstAppMgr)
  {
    LogMsg(LogPrioCritical, "ERROR: TestAppManager::CreateTestAppManager failed to create TestAppManager. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  return pTstAppMgr;
}

TestAppManager::TestAppManager(const int nodeId,const int backupNodeId,ARF &arf,std::string_view backupNodeIpAddr, const unsigned int period, const unsigned int bytes) : 
    m_nodeId{nodeId},m_arf{arf},m_backupNodeIpAddr{backupNodeIpAddr},m_backupNodeId{(unsigned int)backupNodeId}
{
  // appId, primaryNodeId,periodInMs, bytesToSyncEachPeriod, isPrimary,pApp
  m_appsAndInfo[1] = {1,1,period,bytes,false,nullptr};
  m_appsAndInfo[2] = {2,1,period,bytes,false,nullptr};
  m_appsAndInfo[3] = {3,2,period,bytes,false,nullptr};
  m_appsAndInfo[4] = {4,2,period,bytes,false,nullptr};
  
}

void TestAppManager::StartApps()
{
  for(auto &entry : m_appsAndInfo)
  {
    if(entry.second.pApp)
    {
      entry.second.pApp->Start(entry.second.isPrimary);
      OSATaskSleep(5000);
    }
  }
}

void TestAppManager::CreateApps()
{
  
  for(auto & entry : m_appsAndInfo )
  {
    bool isPrimaryNodeForThisApp = entry.second.primaryNodeId == m_nodeId;
    bool isBackupNodeForThisApp  = m_nodeId == m_backupNodeId;
    if(isPrimaryNodeForThisApp || isBackupNodeForThisApp)
    {
      entry.second.isPrimary = isPrimaryNodeForThisApp;
      entry.second.pApp = std::make_unique<FirstSimpleTestApp>(entry.second.appId,"not used",m_backupNodeIpAddr,m_arf, entry.second.periodInMs,m_nodeId, entry.second.bytesToSyncEachPeriod);
      if(!entry.second.pApp )
      {
        LogMsg(LogPrioCritical, "ERROR: TestAppManager::CreateApps failed to create FirtSimpleTestApp instance %u. Errno: 0x%x (%s)",entry.second.appId,errnoGet(),strerror(errnoGet()));
      }
    }
  }
}

void TestAppManager::TriggerShutdownOfAppsOnThisNode()
{
  for(auto & entry : m_appsAndInfo )
  {
    if(entry.second.pApp && entry.second.isPrimary)
    {
      entry.second.pApp->InitiateCountdownToShutdown();
    }
  }
}

void TestAppManager::Print() const
{
  LogMsg(LogPrioInfo, "--- >TestAppManager< ---");
  LogMsg(LogPrioInfo,"Number of AppsInfo: %d backupIp: %s", m_appsAndInfo.size(),m_backupNodeIpAddr.c_str());
  for(auto &entry : m_appsAndInfo)
  {
    if(entry.second.pApp)
    {
      LogMsg(LogPrioInfo,"App: %u is %s on this node",entry.second.appId,entry.second.isPrimary ? "PRIMARY" : "BACKUP");
      entry.second.pApp->Print();
    } 
  }
  
  LogMsg(LogPrioInfo, "--- <TestAppManager> ---");
}
