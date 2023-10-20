#ifndef CCM_LEADER_H
#define CCM_LEADER_H

#include "StateBaseLE.h"
#include "ISender.h"
#include <vector>

class Leader : public StateBaseLE
{
public:
    
  Leader(std::vector<std::unique_ptr<ISender>> &senders);
  std::string GetStateName() const override
  {
    return "Leader";
  }

  StateValue GetValue() const override
  {
    return StateValue::Leader;;
  }
  
  StateValue CheckIfToLeaveLeaderState(std::unique_ptr<LeaderElectionMsg> &pMsg,GMM &gmm);

  void HandleActivity(std::unique_ptr<LeaderElectionMsg> &pMsg, StateBaseLE::StateValue &nextState, GMM &gmm) override;
  void Print() const override;
private:
  void PerformFirstTimeLeaderActions(GMM &gmm);
  void PerformLeavingLeaderStateActions(const StateBaseLE::StateValue nextState, GMM &gmm);
  void SendElectionCompleted(GMM &gmm);
  std::vector<std::unique_ptr<ISender>> &m_senders; 
  bool m_isFirstIterationInState{true};
  
};

#endif //CCM_LEADER_H
