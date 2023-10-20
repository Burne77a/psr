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
  void HandleVote(const LeaderElectionMsg &msg, GMM &gmm);
  void HandleElectionStart(const LeaderElectionMsg &msg, GMM &gmm);
  StateValue HandleElectionCompleted(const LeaderElectionMsg &msg, GMM &gmm);
  StateValue CheckMessageTypeAndActAccordingly(const std::unique_ptr<LeaderElectionMsg> &pMsg,GMM &gmm);
  void PerformFirstTimeInStateActionsIfNeeded(GMM &gmm);
  void SendStartElectionToAllMembers(GMM &gmm);
  void RestartElectionPeriodIfTmo(GMM &gmm);
  void RestartElectionPeriod(const unsigned int newViewNumber,GMM &gmm);
  void Vote(GMM &gmm);
  void SendVote(unsigned int voteOnId,GMM &gmm);
  std::vector<std::unique_ptr<ISender>> &m_senders; 
  std::chrono::system_clock::time_point m_startOfElectionPeriod{std::chrono::system_clock::now()};
  bool m_isFirstIterationInState{true};
  bool m_isVoteCastForPeriod{false};
};

#endif //CCM_ELECTING_H
