#ifndef ARF_ASR_STATESTORER_H
#define ARF_ASR_STATESTORER_H


#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include <memory>
#include <string>


class StateStorer
{
  public:
    
    static std::unique_ptr<StateStorer> CreateStateStorer(const unsigned int port, std::string_view backupIp);
    StateStorer(const unsigned int appId,std::unique_ptr<IReceiver> &pReceiver, std::unique_ptr<ISender> &pSender);
    ~StateStorer() = default;
        
    OSAStatusCode Start();
    void Stop();
    void Print() const;
 
  private:        
    void RunStateMachine();
    
    bool RcvMyStorageData();
    void Flush();
    
    OSAStatusCode StorageTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    
    bool m_isRunning{false};
    const uint32_t  m_periodInMs{5U};
    std::unique_ptr<ISender> m_pSender; 
    std::unique_ptr<IReceiver> m_pReceiver;
    const unsigned int m_appId;
    
};

#endif //ARF_ASR_STATESTORER_H
