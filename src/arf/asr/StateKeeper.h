#ifndef ARF_ASR_STATEKEEPER_H
#define ARF_ASR_STATEKEEPER_H
#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include "StateDataMsg.h"
#include <memory>
#include <string>

class StateKeeper
{
  public:
    
    static std::unique_ptr<StateKeeper> CreateStateKeeper(const unsigned int appId, const unsigned int portRetriever,const unsigned int portKeeper, std::string_view backupIp);
    StateKeeper(const unsigned int appId,std::unique_ptr<ISender> &pSender, std::unique_ptr<IReceiver> &pReceiver);
    ~StateKeeper() = default;
        
    OSAStatusCode Start();
    void Stop();
    void Print() const;
 
  private:        
    void RunStateMachine();
    
    void StoreState(StateDataMsg &stateDataMsg);
    void SendLatestStateToRequester(StateDataMsg &stateDataMsg);
    
    void RcvMyStorageData();
    void Flush();
    
    OSAStatusCode StorageTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    
    bool m_isRunning{false};
    const uint32_t  m_periodInMs{5U};
    std::unique_ptr<ISender> m_pSender; 
    std::unique_ptr<IReceiver> m_pReceiver; 
    const unsigned int m_appId;
    std::unique_ptr<StateDataMsg> m_pLatestStateDataMsg{nullptr};
};

#endif //ARF_ASR_STATEKEEPER_H
