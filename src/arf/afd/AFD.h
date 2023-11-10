#ifndef ARF_AFD_H
#define ARF_AFD_H


#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include <memory>
#include <string>
#include <functional>
#include <chrono>
#include <mutex>

using PrimaryHbTimeoutCb = std::function<void(const unsigned int appId)>;


class AFD
{
  public:
    enum class StateValue
    {
      Waiting = 0,
      BackupWaiting,
      Backup,
      Primary
    };
    
    enum class EventValue
     {
       None = 0,
       BecomePrimary,
       BecomeBackup
     };
    
    
    static std::unique_ptr<AFD> CreateAFD(const unsigned int appId, std::string_view primaryIp, std::string_view backupIp, 
        unsigned int hbTmoTimeInMs, PrimaryHbTimeoutCb hbTmoCb);
    AFD(const unsigned int appId, std::unique_ptr<ISender> &pSender, std::unique_ptr<IReceiver> &pReceiver,const unsigned int hbTmoTimeInMs,PrimaryHbTimeoutCb hbTmoCb);
    ~AFD() = default;
    
    
    void Kickwatchdog();
    
    
    //State commanding methods intended to be used the by app. to set the first role.
    void SetStatePrimary(){SetPendingEvent(EventValue::BecomePrimary);}
    void SetStateBackup() {SetPendingEvent(m_pendingEvent = EventValue::BecomeBackup);}
    
    OSAStatusCode Start();
    void Stop();
    void Print() const;
 
  private:
        
    void WaitingStateHandling();
    void BackupWaitingStateHandling();
    void BackupStateHandling();
    void PrimaryStateHandling();
    void RunStateMachine();
    
    bool HasHeartbeatTmo();
    
    bool SendHeartbeat();
    bool IsHeartbeatRcvd();
    bool RcvMyHb();
    void Flush();
    
    void SetPendingEvent(const EventValue event);
    EventValue GetPendingEvent();
    void SetNewState(const StateValue newState, const EventValue event );
    OSAStatusCode FailureDetectionTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    static const std::string GetStateAsString(const StateValue state);
    static const std::string GetEventAsString(const EventValue event);
    
    bool m_isRunning{false};
    uint32_t  m_periodInMs{10U};
    const unsigned int m_appId;
    std::unique_ptr<ISender> m_pSender; 
    std::unique_ptr<IReceiver> m_pReceiver;
    const unsigned int m_hbTmoTimeInMs;
    StateValue m_currentState{StateValue::Waiting};
    EventValue m_pendingEvent{EventValue::None};
    std::chrono::system_clock::time_point m_lastHbRcvd;
    const PrimaryHbTimeoutCb m_hbTmoCb;
    mutable std::mutex m_eventMutex;
};

#endif //ARF_AFD_H
