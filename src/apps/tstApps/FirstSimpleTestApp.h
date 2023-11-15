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
        unsigned int nodeId);
    ~FirstSimpleTestApp();
    void Start(const bool asPrimary) override;
    void Print() const override;
  private:
    //Called by ARF (AFD) when a the backup do no longer observe heartbeats from primary as expected. 
    void PrimaryHbTimeoutCb(const unsigned int appId);
    void RunStateMachine();
    void SendState();
    void GetStateData();
    OSAStatusCode AppTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    ARF &m_arf;
    bool m_isRunning{false};
    const uint32_t  m_periodInMs{10U};
    bool m_isPrimary{false};
    bool m_nextIsPrimary{false};
    const unsigned int m_appId;
    const std::string m_primaryIpAddr;
    const std::string m_backupIpAddr;
    const unsigned int m_nodeId;
};

#endif //APPS_FIRSTSIMPLETESTAPP_H
