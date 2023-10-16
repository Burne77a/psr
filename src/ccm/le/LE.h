#ifndef CCM_LE_H
#define CCM_LE_H

#include "TaskAbstraction.h"
#include "../gmm/GMM.h"
#include <memory>

class LE 
{
  public:
    LE(GMM &gmm);
    ~LE();
    OSAStatusCode Start();
    void Stop();
    
    std::string GetCurrentStateName() const;
    StateBaseLE::StateValue GetCurrentStateValue() const;
    void SetState(std::unique_ptr<StateBaseLE> newState);
    void HandleActivity();

  private:
    OSAStatusCode LeaderElectionTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    bool m_isRunning{false};
    uint32_t  m_periodInMs{1000U};
    GMM & m_gmm;
    std::unique_ptr<StateBaseLE> m_currentState;
};

#endif //CCM_LE_H
