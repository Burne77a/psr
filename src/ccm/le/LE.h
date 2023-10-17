#ifndef CCM_LE_H
#define CCM_LE_H

#include "StateBaseLE.h"
#include "TaskAbstraction.h"
#include "ISender.h"
#include "IReceiver.h"
#include "../gmm/GMM.h"
#include <memory>
#include <string>

class LE 
{
  public:
    static std::unique_ptr<LE> CreateLE(GMM & gmm);
    LE(GMM &gmm,std::vector<std::unique_ptr<ISender>> &senders, std::unique_ptr<IReceiver> &pReceiver);
    ~LE();
  
    std::string GetCurrentStateName() const;
    StateBaseLE::StateValue GetCurrentStateValue() const;
    void SetState(std::unique_ptr<StateBaseLE> newState);
    void HandleActivity();
    void Print();

  private:
    static void LogStateChange(const std::string& from, const std::string& to);
    static std::unique_ptr<StateBaseLE> CreateState(StateBaseLE::StateValue stateValue);
    OSAStatusCode LeaderElectionTaskMethod();
    static OSAStatusCode ClassTaskMethod(void * const pInstance);
    GMM & m_gmm;
    std::unique_ptr<StateBaseLE> m_currentState;
    std::vector<std::unique_ptr<ISender>> m_senders; 
    std::unique_ptr<IReceiver> m_pReceiver;
};

#endif //CCM_LE_H
