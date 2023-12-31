#ifndef APPS_FIRSTSIMPLETESTAPP_H
#define APPS_FIRSTSIMPLETESTAPP_H
#include "../IApp.h"
#include "../../arf/ARF.h"
#include "TaskAbstraction.h"
#include <string>
class FirstSimpleTestApp : public IApp
{
  public:
    FirstSimpleTestApp(unsigned int appId,std::string_view primaryIpAddr, std::string_view backupIpAddr, ARF& arf, unsigned int periodInMs,
        unsigned int nodeId, const unsigned int bytes);
    ~FirstSimpleTestApp();
    void Start(const bool asPrimary) override;
    void InitiateCountdownToShutdown() override;
    void Print() const override;
  private:
    //Called by ARF (AFD) when a the backup do no longer observe heartbeats from primary as expected. 
    void PrimaryHbTimeoutCb(const unsigned int appId);
    void RunStateMachine();
    void SendState();
    void GetStateData();
    void HandleCountdownToShutdownIfActive();
    OSAStatusCode AppTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    ARF &m_arf;
    bool m_isRunning{false};
    const uint32_t  m_periodInMs;
    const unsigned int m_bytesToSync;
    bool m_isPrimary{false};
    bool m_isCountDownStartRequested{false};
    bool m_isCountDownRunning{false};
    bool m_nextIsPrimary{false};
    const unsigned int m_appId;
    const std::string m_primaryIpAddr;
    const std::string m_backupIpAddr;
    const unsigned int m_nodeId;
    unsigned int m_iterationCount{1000000U};
    const unsigned int m_shutdownLimit{100U};
};

#endif //APPS_FIRSTSIMPLETESTAPP_H
