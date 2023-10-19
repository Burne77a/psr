#ifndef CCM_FOLLOWER_H
#define CCM_FOLLOWER_H

#include "StateBaseLE.h"
#include <chrono>

class Follower : public StateBaseLE
{
  public:
    Follower();
    std::string GetStateName() const override
    {
      return "Follower";
    }
  
    StateValue GetValue() const override
    {
      return StateValue::Follower;
    }
  
    void HandleActivity(std::unique_ptr<LeaderElectionMsg> &pMsg, StateBaseLE::StateValue &nextState, GMM &gmm)  override;
    
    void Print() const override;
  
  private:
    StateValue CheckForAndHandleLeaderLoss(GMM &gmm);
    std::chrono::system_clock::time_point m_lastLeader;
  
};


#endif //CCM_FOLLOWER_H
