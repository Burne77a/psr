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
  public:
    static std::unique_ptr<TestAppManager> CreateTestAppManager(const int nodeId,ARF &arf,std::string_view backupNodeIpAddr);
    TestAppManager(const int nodeId,ARF &arf,std::string_view backupNodeIpAddr);
    ~TestAppManager() = default;
    void CreateApps();
    void StartApps(const bool asPrimary);
    void Print() const;      
  private:
    const int m_nodeId;
    ARF &m_arf;
    const std::string m_backupNodeIpAddr;
    std::unordered_map<unsigned int, std::unique_ptr<IApp>> m_tstApps;
};

#endif //APPS_TESTAPPMGR_H
