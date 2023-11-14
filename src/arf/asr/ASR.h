#ifndef ARF_ASR_H
#define ARF_ASR_H


#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include <memory>
#include <string>


class ASR
{
  public:
    
    static std::unique_ptr<ASR> CreateASR(const unsigned int appId);
    ASR(const unsigned int appId, std::unique_ptr<IReceiver> &pReceiver);
    ~ASR() = default;
    
    bool SendToStorage(const ISerializable & objToSend);
    
    bool GetFromStorage(const ISerializable & objToRcvTo);
    
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
    const uint32_t  m_periodInMs{10U};
    const unsigned int m_appId;
    std::unique_ptr<ISender> m_pSender; 
    std::unique_ptr<IReceiver> m_pReceiver;
    
};

#endif //ARF_ASR_H
