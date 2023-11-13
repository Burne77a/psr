#include "TestAppManager.h"
#include "tstApps/FirstSimpleTestApp.h"
#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<TestAppManager> TestAppManager::CreateTestAppManager(const int nodeId,ARF &arf, std::string_view backupNodeIpAddr)
{
  std::unique_ptr<TestAppManager> pTstAppMgr = std::make_unique<TestAppManager>(nodeId,arf,backupNodeIpAddr);
  if(!pTstAppMgr)
  {
    LogMsg(LogPrioCritical, "ERROR: TestAppManager::CreateTestAppManager failed to create TestAppManager. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  return pTstAppMgr;
}

TestAppManager::TestAppManager(const int nodeId,ARF &arf,std::string_view backupNodeIpAddr) : m_nodeId{nodeId},m_arf{arf},m_backupNodeIpAddr{backupNodeIpAddr}
{
  
}

void TestAppManager::StartApps(const bool asPrimary)
{
  for(auto &entry : m_tstApps)
  {
    if(entry.second)
    {
      entry.second->Start(asPrimary);
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: TestAppManager::StartApps invalid pointer find in map Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    }
  }
}

void TestAppManager::CreateApps()
{
  if(m_nodeId == 1 || m_nodeId == 3)
  {
    const unsigned int appId = 1;
    std::unique_ptr<IApp> pApp = std::make_unique<FirstSimpleTestApp>(appId,"not used",m_backupNodeIpAddr,m_arf, 10,m_nodeId);
    if(pApp)
    {
      m_tstApps.insert(std::make_pair(appId, std::move(pApp)));
    
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: TestAppManager::CreateApps failed to create FirtSimpleTestApp. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    }
  }
}

void TestAppManager::Print() const
{
  LogMsg(LogPrioInfo, "--- >TestAppManager< ---");
  LogMsg(LogPrioInfo,"Number of Apps: %d backupIp: %s", m_tstApps.size(),m_backupNodeIpAddr.c_str());
  for(auto &entry : m_tstApps)
  {
    if(entry.second)
    {
      entry.second->Print();
    }
  }
  
  LogMsg(LogPrioInfo, "--- <TestAppManager> ---");
}
