#ifndef APPS_FIRSTSIMPLETESTAPP_H
#define APPS_FIRSTSIMPLETESTAPP_H
#include "../IApp.h"
#include "../../arf/ARF.h"
#include "TaskAbstraction.h"
#include <string>
class FirstSimpleTestApp : public IApp
{
  public:
    FirstSimpleTestApp(unsigned int appId,std::string_view primaryIpAddr, std::string_view backupIpAddr, ARF& arf, unsigned int periodInMs);
    ~FirstSimpleTestApp();
    void Start(const bool asPrimary) override;
    void Print() const override;
  private:
    void RunStateMachine();
    OSAStatusCode AppTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    ARF &m_arf;
    bool m_isRunning{false};
    const uint32_t  m_periodInMs{10U};
    bool m_isPrimary{false};
    const unsigned int m_appId;
    const std::string m_primaryIpAddr;
    const std::string m_backupIpAddr;
};

#endif //APPS_FIRSTSIMPLETESTAPP_H
