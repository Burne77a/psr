#ifndef CCM_CSA_H
#define CCM_CSA_H

#include "ServiceUpcallDispatcher.h"
#include "LeaderCommunicator.h"
#include "ClientRequestId.h"
#include "../gmm/GMM.h"
#include "../ICCM.h"
#include <memory>
#include <mutex>
#include <string>

class CSA
{
  public:
    static std::unique_ptr<CSA> CreateCSA(const std::string_view leaderIp, GMM& gmm);
    CSA(GMM& gmm, std::unique_ptr<ServiceUpcallDispatcher> & pSrvDispatcher, std::unique_ptr<LeaderCommunicator>& pLeaderComm);
    ~CSA() = default;
    
    bool GetClientRequestsSentToLeader(ClientMessage & msg);
    void FlushOutMessageToLeader() {m_pLeaderComm->FlushMsgToLeader();}
    
    bool SendReplyToClient(ClientMessage & msg);
    
    void MakeUpcall(std::shared_ptr<ClientMessage> pCmsg) 
    {
       m_pSrvDispatcher->MakeUpcall(pCmsg);
    }
    

    bool ReplicateRequest(const ClientMessage & msg) ;
    bool RegisterService(const unsigned int serviceId,UpcallCallbackType upcallCb)
    {
      return m_pSrvDispatcher->RegisterService(serviceId, upcallCb);
    }
    ClientRequestId CreateUniqueId();
    
    void Print() const;
  private:
    bool SendToLeaderAndWaitForReply(const ClientMessage &msg);
    GMM& m_gmm;
    std::unique_ptr<ServiceUpcallDispatcher> m_pSrvDispatcher;
    std::unique_ptr<LeaderCommunicator> m_pLeaderComm;
   mutable std::mutex m_oneReqAtTheTimeMutex{};
};

#endif //CCM_CSA_H
