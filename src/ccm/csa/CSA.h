#ifndef CCM_CSA_H
#define CCM_CSA_H
#include "../CCM.h"
#include "ServiceUpcallDispatcher.h"
#include "LeaderCommunicator.h"
#include "ClientRequestId.h"
#include <memory>
#include <mutex>
class CSA
{
  public:
    static std::unique_ptr<CSA> CreateCSA(const int myId);
    CSA(std::shared_ptr<CCM> &pCcm, std::unique_ptr<ServiceUpcallDispatcher> & pSrvDispatcher, std::unique_ptr<LeaderCommunicator>& pLeaderComm);
    ~CSA() = default;
    const OSAStatusCode Start();
    bool ReplicateRequest(const ClientMessage & msg);
    bool RegisterService(const unsigned int serviceId,UpcallCallbackType upcallCb) 
    {
      std::lock_guard<std::mutex> lock(m_mutex); 
      return m_pSrvDispatcher->RegisterService(serviceId, upcallCb);
    }
    void Print() const;
    ClientRequestId CreateUniqueId();
  private:
    bool SendToLeaderAndWaitForReply(const ClientMessage &msg);
    std::shared_ptr<CCM> m_pCcm;
    std::unique_ptr<ServiceUpcallDispatcher> m_pSrvDispatcher;
    std::unique_ptr<LeaderCommunicator> m_pLeaderComm;
   mutable std::mutex m_mutex{};
};

#endif //CCM_CSA_H
