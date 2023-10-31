#ifndef CCM_LEADERCOMMUNICATOR_H
#define CCM_LEADERCOMMUNICATOR_H

#include "IReceiver.h"
#include "ISender.h"
#include "ClientMessage.h"
#include "../gmm/GMM.h"
#include <memory>
#include <vector>

class LeaderCommunicator
{
  public: 
    static std::unique_ptr<LeaderCommunicator> CreateLeaderCommunicator(const std::string_view leaderIpAddress, const int leaderRcvPort,GMM & gmm, const int leaderSndPort);
    
    LeaderCommunicator(std::unique_ptr<ISender>& pLeaderSnd, std::unique_ptr<IReceiver>&  pLeaderRcv, std::vector<std::unique_ptr<ISender>>& clientSenders, std::unique_ptr<IReceiver>& pClientRcv);
    ~LeaderCommunicator() = default;
    bool WaitForCommitAckToRequest(const ClientMessage &req,const unsigned int timeToWaitInMs);
    bool SendToLeaderWithRetries(const ClientMessage &msg, const unsigned int retries, const unsigned int timeBetweenInMs);
    bool GetClientRequestsSentToLeader(ClientMessage & msg);
    bool SendToClient(ClientMessage & msg, const GMM & gmm);
    
    void FlushMsgToLeader();
    
 
  private:
    bool ReceiveFromClient(ClientMessage &msg);
    bool SendToLeader(const ClientMessage &msg);
    bool ReceiveFromLeader(ClientMessage &rcvdMsg);
    bool IsValidReply(const ClientMessage &req,const ClientMessage &reply);
    //One port for client to leader communication, that is the m_pLeaderRcv and m_pLeaderSnd. The leader receives using m_pLeaderRcv, the client sends using m_pLeaderSnd
    //For sending replies, the m_clientSenders contain a list of senders to all clients (different port) The leader uses that to send request replies to the client.
    //The client listens for replies using the m_pClientRcv.
    std::unique_ptr<IReceiver> m_pLeaderRcv;
    std::unique_ptr<ISender> m_pLeaderSnd;
    std::vector<std::unique_ptr<ISender>> m_clientSenders; 
    std::unique_ptr<IReceiver> m_pClientRcv;
    
};

#endif //CCM_LEADERCOMMUNICATOR_H
