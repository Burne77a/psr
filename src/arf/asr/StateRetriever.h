#ifndef ARF_ASR_STATERETRIEVER_H
#define ARF_ASR_STATERETRIEVER_H
#include "ISender.h"
#include "IReceiver.h"
#include "StateDataMsg.h"
#include <memory>
#include <string>

class StateRetriever
{
  public:
    
    static std::unique_ptr<StateRetriever> CreateStateRetriever(const unsigned int appId, const unsigned int port, std::string_view storageIp);
    StateRetriever(const unsigned int appId,std::unique_ptr<ISender> &pSender, std::unique_ptr<IReceiver> &pReceiver);
    ~StateRetriever() = default;
    
    bool GetLatestStateFromStorage(ISerializable & objToPopulate);
    void Print() const;
  private:
    bool PopulateFromRcvdState(const StateDataMsg &msgRcvd, ISerializable & objToPopulate);
    bool WaitForReply(StateDataMsg &msgRcvd);
    std::unique_ptr<ISender> m_pSender; 
    std::unique_ptr<IReceiver> m_pReceiver;   
    const unsigned int m_appId;
};

#endif //ARF_ASR_STATERETRIEVER_H
