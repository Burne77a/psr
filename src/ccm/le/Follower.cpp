#include "Follower.h"
#include "Logger.h"
Follower::Follower() : m_lastLeader(std::chrono::system_clock::now())
{
  
}
void Follower::HandleActivity(StateBaseLE::StateValue &nextState, GMM &gmm) 
{
  nextState = CheckForAndHandleLeaderLoss(gmm);
}

StateBaseLE::StateValue Follower::CheckForAndHandleLeaderLoss(GMM &gmm)
{
  static const std::chrono::milliseconds leaderTimeoutTimeInMs(3000);
  const auto now = std::chrono::system_clock::now();
  const auto elapsed = now - m_lastLeader;
  if(gmm.IsLeaderAvailable())
  {
    m_lastLeader = std::chrono::system_clock::now();
  }
  else
  {
    if(elapsed > leaderTimeoutTimeInMs)
    {
      LogMsg(LogPrioInfo,"Follower lost leader entering electing.");
      return StateValue::Electing;
    }
  }
  return StateValue::Follower;   
}

void Follower::Print() const
{
  const auto now = std::chrono::system_clock::now();
  const auto elapsed = now - m_lastLeader;
  LogMsg(LogPrioInfo, "State Follower. Time since leader was available: %s",std::to_string(elapsed.count()).c_str());
}

