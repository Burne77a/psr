#include "Follower.h"
#include "LeaderElectionMsg.h"
#include "Logger.h"
Follower::Follower() : m_lastLeader(std::chrono::system_clock::now())
{
  
}
void Follower::HandleActivity(std::unique_ptr<LeaderElectionMsg> &pMsg, StateBaseLE::StateValue &nextState, GMM &gmm) 
{
  gmm.ResetLeaderVoteCount();
  
  if(pMsg)
  {
    if(pMsg->IsMsgViewNumberHigherThanViewNumberWhenRcvd())
    {
      if(pMsg->IsVoteMsg())
      {
        gmm.AddLeaderVote(pMsg->GetIdOfVoteDst());
        gmm.SetMyViewNumber(pMsg->GetViewNumber());
        LogMsg(LogPrioInfo,"Follower got a valid leader vote, entering electing. ");
        pMsg->Print();
        nextState = StateValue::Electing;
      }
      else if(pMsg->IsElectionCompletedMsg())
      {
        LogMsg(LogPrioInfo,"Follower got a valid election start, entering electing. ");
        gmm.SetMyViewNumber(pMsg->GetViewNumber());
        pMsg->Print();
        nextState = StateValue::Electing;
      }
      else
      {
        LogMsg(LogPrioInfo,"Follower got a valid leader election message, but stays in follower state");
        pMsg->Print();
      }
    }
    else
    {
      LogMsg(LogPrioInfo,"Follower got a leader election message that is ignored. Most likely to low view number");
      pMsg->Print();
    }
  }
  else
  {
    nextState = CheckForAndHandleLeaderLoss(gmm);
  }
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
      if(gmm.IsAnyMemberQuorumConnected())
      {
        gmm.SetMyViewNumber(gmm.GetLargestViewNumber() + 1U); 
        
        LogMsg(LogPrioInfo,"Follower lost leader entering and QC member exist, entering electing. Setting view number to %u", gmm.GetMyViewNumber());
        return StateValue::Electing;
      }      
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

