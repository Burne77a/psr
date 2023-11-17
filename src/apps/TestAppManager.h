#ifndef APPS_TESTAPPMGR_H
#define APPS_TESTAPPMGR_H
#include "IApp.h"
#include "../arf/ARF.h"
#include "TaskAbstraction.h"
#include <string>
#include <memory>
#include <unordered_map>

class TestAppManager
{
  private:
    struct TestAppInfo
    {
      unsigned int appId{0U};
      unsigned int primaryNodeId{0U};
      unsigned int periodInMs{0U};
      unsigned int bytesToSyncEachPeriod{0U};
      bool isPrimary{false};
      std::unique_ptr<IApp> pApp{nullptr};
    };
  public:
    static std::unique_ptr<TestAppManager> CreateTestAppManager(const int nodeId,const int backupNodeId, ARF &arf,std::string_view backupNodeIpAddr);
    TestAppManager(const int nodeId,const int backupNodeId,ARF &arf,std::string_view backupNodeIpAddr);
    ~TestAppManager() = default;
    void CreateApps();
    void TriggerShutdownOfAppsOnThisNode();
    void StartApps();
    void Print() const;      
  private:
    const int m_nodeId;
    ARF &m_arf;
    const std::string m_backupNodeIpAddr;
    const unsigned int m_backupNodeId;
    std::unordered_map<unsigned int, TestAppInfo> m_appsAndInfo;
};

#endif //APPS_TESTAPPMGR_H
