#ifndef CCM_ELECTING_H
#define CCM_ELECTING_H

#include "StateBaseLE.h"
#include "ISender.h"
#include <vector>
#include <memory>
#include <chrono>

class Electing : public StateBaseLE
{
public:
  Electing(std::vector<std::unique_ptr<ISender>> &senders);  
    
  std::string GetStateName() const override
  {
    return "Electing";
  }

  StateValue GetValue() const override
  {
    return StateValue::Electing;
  }

  void HandleActivity(std::unique_ptr<LeaderElectionMsg> &pMsg, StateBaseLE::StateValue &nextState, GMM &gmm)  override;
  
  void Print() const override;
private:
  void HandleVotes(const LeaderElectionMsg &msg,GMM &gmm);
  void CheckMessageTypeAndActAccordingly(const std::unique_ptr<LeaderElectionMsg> &pMsg,GMM &gmm);
  void PerformFirstTimeInStateActionsIfNeeded(GMM &gmm);
  void SendStartElectionToAllMembers(GMM &gmm);
  void RestartElectionPeriod(GMM &gmm);
  std::vector<std::unique_ptr<ISender>> &m_senders; 
  std::chrono::system_clock::time_point m_startOfElectionPeriod{std::chrono::system_clock::now()};
  bool m_isFirstTimeInState{true};
};

#endif //CCM_ELECTING_H
