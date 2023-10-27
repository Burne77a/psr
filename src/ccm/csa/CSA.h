#ifndef CCM_CSA_H
#define CCM_CSA_H
#include "../CCM.h"
#include "ServiceUpcallDispatcher.h"
#include "ClientRequestId.h"
#include <memory>
#include <mutex>
class CSA
{
  public:
    static std::unique_ptr<CSA> CreateCSA(const int myId);
    CSA(std::shared_ptr<CCM> &pCcm, std::unique_ptr<ServiceUpcallDispatcher> & pSrvDispatcher);
    ~CSA() = default;
    const OSAStatusCode Start();
    bool ReplicateRequest(const ClientMessage & msg);
    bool RegisterService(const unsigned int serviceId,UpcallCallbackType upcallCb) {return m_pSrvDispatcher->RegisterService(serviceId, upcallCb);}
    void Print() const;
    ClientRequestId CreateUniqueId();
  private:
    bool SendToLeader(const ClientMessage &msg);
    std::shared_ptr<CCM> m_pCcm;
    std::unique_ptr<ServiceUpcallDispatcher> m_pSrvDispatcher;
   mutable std::mutex m_mutex{};
};

#endif //CCM_CSA_H
