#include "Leader.h"
#include "LeaderElectionMsg.h"
#include "Logger.h"

Leader::Leader(std::vector<std::unique_ptr<ISender>> &senders) : m_senders(senders) //Requires that the lifespan of the senders are longer than that of Leader. 
{
  
}

void Leader::HandleActivity(std::unique_ptr<LeaderElectionMsg> &pMsg, StateBaseLE::StateValue &nextState, GMM &gmm) 
{
  PerformFirstTimeLeaderActions(gmm);
  
  nextState = CheckIfToLeaveLeaderState(pMsg,gmm); 
}

StateBaseLE::StateValue Leader::CheckIfToLeaveLeaderState(std::unique_ptr<LeaderElectionMsg> &pMsg,GMM &gmm)
{
  StateValue nextState = StateValue::Leader;
  if(pMsg)
  {
    const unsigned int rcvdViewNumber = pMsg->GetViewNumber();
    const unsigned int myViewNumber = gmm.GetMyViewNumber();
    if(rcvdViewNumber > myViewNumber)
    {
      LogMsg(LogPrioWarning, "WARNING: Leader::CheckIfToLeaveLeaderState received message with higher view number %u > %u",rcvdViewNumber,myViewNumber);
      pMsg->Print();
      nextState = StateValue::Follower;
    }
  }
  if(!gmm.IsQuorumConnected(gmm.GetMyId()))
  {
    LogMsg(LogPrioWarning, "WARNING: Leader::CheckIfToLeaveLeaderState no longer QC leaving leader role");
    nextState = StateValue::Follower;
  }
  return nextState;
}

void Leader::PerformFirstTimeLeaderActions(GMM &gmm)
{
  if(m_isFirstIterationInState)
  {
    SendElectionCompleted(gmm);
  }
  m_isFirstIterationInState = false;
}

void Leader::SendElectionCompleted(GMM &gmm)
{
  LeaderElectionMsg leEcMsg(LeaderElectionMsg::MsgType::ElectionCompleted);
  leEcMsg.SetSenderId(gmm.GetMyId());
  leEcMsg.SetViewNumber(gmm.GetMyViewNumber());
  for(auto & pSender : m_senders )
  {
    if(pSender)
    {
      if(!pSender->Send(leEcMsg))
      {
        LogMsg(LogPrioCritical, "ERROR: Leader::SendElectionCompleted Failed to send ElectionStart message");
      }
    }
  }
}

void Leader::Print() const
{
  LogMsg(LogPrioInfo, "State Leader.");
}
