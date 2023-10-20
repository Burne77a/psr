#ifndef CCM_FD_H
#define CCM_FD_H

#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include "HeartbeatCCM.h"
#include "../gmm/GMM.h"
#include <vector>
#include <memory>

class FD
{
  public:
    static std::unique_ptr<FD> CreateFD(GMM & gmm);
    FD(GMM & gmm, std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver);
    ~FD();
    OSAStatusCode Start();
    void Stop();
 
  private:
   
    void PopulateAndSendHeartbeat();
    void Populate(HeartbeatCCM &heartbeat);
    void Send(HeartbeatCCM &heartbeat);
    void HandleIncommingHeartbeat();
    void UpdateMemberWithReceivedHeartbeatData(const HeartbeatCCM &rcvdHeartbeat);
    void UpdateMySelfWithReceivedHeartbeatData(const HeartbeatCCM &rcvdHeartbeat);
    void CheckForFailingMembersAndUpdate();
    OSAStatusCode FailureDetectionTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    bool m_isRunning{false};
    uint32_t  m_periodInMs{200U};
    GMM & m_gmm;
    std::vector<std::unique_ptr<ISender>> m_senders; 
    std::unique_ptr<IReceiver> m_pReceiver;
};

#endif //CCM_FD_H
