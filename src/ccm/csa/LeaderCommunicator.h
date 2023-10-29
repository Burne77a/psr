#ifndef CCM_LEADERCOMMUNICATOR_H
#define CCM_LEADERCOMMUNICATOR_H

#include "IReceiver.h"
#include "ISender.h"
#include "ClientMessage.h"
#include <memory>

class LeaderCommunicator
{
  public: 
    static std::unique_ptr<LeaderCommunicator> CreateLeaderCommunicator(const std::string_view leaderIpAddress, const int port);
    LeaderCommunicator(std::unique_ptr<ISender>& pSender, std::unique_ptr<IReceiver>&  pReceiver);
    ~LeaderCommunicator() = default;
    bool WaitForReplyToRequest(const ClientMessage &msg,const unsigned int timeToWaitInMs);
    bool SendToLeaderWithRetries(const ClientMessage &msg, const unsigned int retries, const unsigned int timeBetweenInMs);
    bool SendToLeader(const ClientMessage &msg);
  private:
#warning Create receiver to replies, and sender of replies
    std::unique_ptr<IReceiver> m_pReceiver;
    std::unique_ptr<ISender> m_pSender;
};

#endif //CCM_LEADERCOMMUNICATOR_H
