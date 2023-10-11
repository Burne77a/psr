#ifndef CCM_FD_H
#define CCM_FD_H

#include "TaskAbstraction.h"
#include "../gmm/GMM.h"

class FD
{
  public:
    FD(GMM & gmm);
    ~FD();
    OSAStatusCode Start();
    void Stop();
    
  private:
    void PopulateAndSendHeartbeat();
    void Populate();
    void Send();
    void HandleIncommingHeartbeat();
    OSAStatusCode FailureDetectionTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    bool m_isRunning{false};
    uint32_t  m_periodInMs{1000U};
    GMM & m_gmm;
};

#endif //CCM_FD_H
