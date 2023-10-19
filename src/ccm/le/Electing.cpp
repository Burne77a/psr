#include "Electing.h"
#include "LeaderElectionMsg.h"
#include "Logger.h"

Electing::Electing(std::vector<std::unique_ptr<ISender>> &senders) : m_senders(senders) //Requires that the lifespan of the senders are longer than that of Electing. 
{
  
}

void Electing::HandleActivity(std::unique_ptr<LeaderElectionMsg> &pMsg, StateBaseLE::StateValue &nextState, GMM &gmm) 
{
  PerformFirstTimeInStateActionsIfNeeded();
  
  CheckMessageTypeAndActAccordingly();
  
  
  if(gmm.IsVoteCntForMySelfLargerThanMajority())
  {
    //Become leader
  }
  else
  {
    //restart election period if needed.
  }
}


void Electing::HandleVote(const LeaderElectionMsg &msg,GMM &gmm)
{
  const unsigned int msgViewNumber = pMsg->GetViewNumber();
  const unsigned int myViewNumber = gmm.GetMyViewNumber();
  const unsigned int voteAddressedToId = msg.GetIdOfVoteDst();
  const unsigned int voteFromId = msg.GetSenderId();
  
  if(msgViewNumber > myViewNumber) //There is a higher view out there..
  {
    LogMsg(LogPrioInfo, "Electing::HandleVote vote with higher view number received, restarting election period %u %u %u --> %u",msgViewNumber,myViewNumber,voteFromId,voteAddressedToId);
    RestartElectionPeriod(msgViewNumber,gmm);
  }
  
  if(msgViewNumber == myViewNumber) 
  {  
    if(voteAddressedToId == gmm.GetMyId()) //if the observed vote was for this node, count it. Count it even if the election period was just restarted. 
    {
      LogMsg(LogPrioInfo, "Electing::HandleVote received valid vote from %u (view %u) ",voteFromId,msgViewNumber);
    }
    else
    {
      LogMsg(LogPrioInfo, "Electing::HandleVote received valid vote destined to another member %u from %u (view %u) ",voteAddressedToId,voteFromId,msgViewNumber);
    }
    gmm.AddLeaderVote(voteAddressedToId);
  }
  else
  {
    LogMsg(LogPrioInfo, "Electing::HandleVote with to low view number %u %u %u --> %u",msgViewNumber,myViewNumber,voteFromId,voteAddressedToId);
  }
}

void Electing::CheckMessageTypeAndActAccordingly(const std::unique_ptr<LeaderElectionMsg> &pMsg,GMM &gmm)
{
  if(pMsg)
  {
    const unsigned int msgViewNumber = pMsg->GetViewNumber();
    const unsigned int myViewNumber = gmm.GetMyViewNumber();
    if(pMsg->IsVoteMsg())
    {
      HandleVote(*pMsg, gmm);
    }
    else if(pMsg->IsElectionStartMsg())
    {
      
    }
  }
}

void Electing::PerformFirstTimeInStateActionsIfNeeded(GMM &gmm)
{
  if(m_isFirstTimeInState)
  {
    LogMsg(LogPrioInfo, "Electing::PerformFirstTimeInStateActionsIfNeeded entered Election state - performing first actions");
    SendStartElectionToAllMembers(gmm);
    
    //Cast vote
    
    m_isFirstTimeInState = false;
  }
}

void Electing::SendStartElectionToAllMembers(GMM &gmm)
{
  LeaderElectionMsg leMsg(LeaderElectionMsg::MsgType::ElectionStart);
  leMsg.SetSenderId(gmm.GetMyId());
  leMsg.SetViewNumber(gmm.GetMyViewNumber());
  
  LogMsg(LogPrioInfo, "Electing::PerformFirstTimeInStateActionsIfNeeded sending ElectionStart to all members.");
  
  for(auto & pSender : m_senders )
  {
    if(pSender)
    {
      if(!pSender->Send(leMsg))
      {
        LogMsg(LogPrioCritical, "ERROR: Electing::SendStartElectionToAllMembers Failed to send ElectionStart message");
      }
    }
  }
}


void Electing::RestartElectionPeriod(const unsigned int newViewNumber,GMM &gmm)
{
  LogMsg(LogPrioInfo, "Electing::RestartElectionPeriod Restarting election period, view number %u",newViewNumber);
  m_startOfElectionPeriod = std::chrono::system_clock::now();
  gmm.ResetLeaderVoteCount();
  gmm.SetMyViewNumber(newViewNumber);
  
  SendStartElectionToAllMembers(gmm);
}

void Electing::Print() const
{
  LogMsg(LogPrioInfo, "State Electing");
}
