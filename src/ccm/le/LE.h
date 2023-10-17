#ifndef CCM_LE_H
#define CCM_LE_H

#include "StateBaseLE.h"
#include "TaskAbstraction.h"
#include "../gmm/GMM.h"
#include <memory>
#include <string>

class LE 
{
  public:
    static std::unique_ptr<LE> CreateLE(GMM & gmm);
    LE(GMM &gmm);
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
};

#endif //CCM_LE_H
