#ifndef CCM_FD_H
#define CCM_FD_H

#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include "HeartbeatCCM.h"
#include "../gmm/GMM.h"
#include <vector>

class FD
{
  public:
    FD(GMM & gmm,ISender & sender, IReceiver receiver);
    ~FD();
    OSAStatusCode Start();
    void Stop();
    
  private:
    void PopulateAndSendHeartbeat();
    void Populate(std::vector<HeartbeatCCM> &heartbeats);
    void Send(std::vector<HeartbeatCCM> &heartbeats);
    void HandleIncommingHeartbeat();
    OSAStatusCode FailureDetectionTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    bool m_isRunning{false};
    uint32_t  m_periodInMs{1000U};
    GMM & m_gmm;
    ISender m_sender; 
    IReceiver m_receiver;
};

#endif //CCM_FD_H
