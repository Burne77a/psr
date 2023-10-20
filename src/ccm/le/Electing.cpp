#include "Electing.h"
#include "LeaderElectionMsg.h"
#include "Logger.h"

Electing::Electing(std::vector<std::unique_ptr<ISender>> &senders) : m_senders(senders) //Requires that the lifespan of the senders are longer than that of Electing. 
{
  
}

void Electing::HandleActivity(std::unique_ptr<LeaderElectionMsg> &pMsg, StateBaseLE::StateValue &nextState, GMM &gmm) 
{
  if(gmm.IsAnyMemberQuorumConnected())
  {
    PerformFirstTimeInStateActionsIfNeeded(gmm);
    Vote(gmm);
    
    nextState = CheckMessageTypeAndActAccordingly(pMsg,gmm);
    if(nextState == StateValue::Electing)
    {
      RestartElectionPeriodIfTmo(gmm);
    }
  }
  else
  {
    //If no instace is QC, no one can become leader. Revert to Follower and wait. 
    nextState = StateValue::Follower;
  }
}


void Electing::HandleVote(const LeaderElectionMsg &msg, GMM &gmm)
{
  const unsigned int msgViewNumber = msg.GetViewNumber();
  const unsigned int myViewNumber = gmm.GetMyViewNumber();
  const unsigned int voteAddressedToId = msg.GetIdOfVoteDst();
  const unsigned int voteFromId = msg.GetSenderId();
  
  if(msgViewNumber > myViewNumber) //There is a higher view out there..
  {
    LogMsg(LogPrioInfo, "Electing::HandleVote vote with higher view number received, restarting election period msgv:%u myv:%u %u --> %u",msgViewNumber,myViewNumber,voteFromId,voteAddressedToId);
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

void Electing::HandleElectionStart(const LeaderElectionMsg &msg, GMM &gmm)
{
  const unsigned int msgViewNumber = msg.GetViewNumber();
  const unsigned int myViewNumber = gmm.GetMyViewNumber();
  const unsigned int msgFromId = msg.GetSenderId();
  if(msgViewNumber > myViewNumber) //There is a higher view out there..
  {
    LogMsg(LogPrioInfo, "Electing::HandleElectionStart ElectionStart with higher view number received, restarting election period msgv:%u myv:%u from: %u",msgViewNumber,myViewNumber,msgFromId);
    RestartElectionPeriod(msgViewNumber,gmm);
  }
  else
  {
    LogMsg(LogPrioInfo, "Electing::HandleElectionStart ElectionStart with lower or equal view number received - ignoring msgv:%u myv:%u from: %u",msgViewNumber,myViewNumber,msgFromId);
  }
}

StateBaseLE::StateValue Electing::HandleElectionCompleted(const LeaderElectionMsg &msg, GMM &gmm)
{
  StateValue nextState = StateValue::Electing;
  const unsigned int msgViewNumber = msg.GetViewNumber();
  const unsigned int myViewNumber = gmm.GetMyViewNumber();
  const unsigned int msgFromId = msg.GetSenderId();
  if(msgViewNumber >= myViewNumber)
  {
    LogMsg(LogPrioInfo, "Electing::HandleElectionCompleted ElectionCompleted with higher view number received, entering follower state msgv:%u myv:%u from: %u",msgViewNumber,myViewNumber,msgFromId);
    nextState = StateValue::Follower;
  }
  else
   {
     LogMsg(LogPrioInfo, "Electing::HandleElectionCompleted ElectionCompleted with lower view number received - ignoring msgv:%u myv:%u from: %u",msgViewNumber,myViewNumber,msgFromId);
   }
  return nextState;
}

StateBaseLE::StateValue Electing::CheckMessageTypeAndActAccordingly(const std::unique_ptr<LeaderElectionMsg> &pMsg, GMM &gmm)
{
  StateValue stateToGoTo = StateValue::Electing;
  if(pMsg)
  {
    if(pMsg->IsVoteMsg())
    {
      HandleVote(*pMsg, gmm);
      if(gmm.IsVoteCntForMySelfLargerThanMajority())
      {
        stateToGoTo = StateValue::Leader;
      }
    }
    else if(pMsg->IsElectionStartMsg())
    {
      HandleElectionStart(*pMsg, gmm);
      
    }
    else if(pMsg->IsElectionCompletedMsg())
    {
      stateToGoTo = HandleElectionCompleted(*pMsg, gmm);
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: Electing::CheckMessageTypeAndActAccordingly Invalid leader election msg type");
      pMsg->Print(); 
    }
  }
  return stateToGoTo;
}

void Electing::PerformFirstTimeInStateActionsIfNeeded(GMM &gmm)
{
  if(m_isFirstIterationInState)
  {
    LogMsg(LogPrioInfo, "Electing::PerformFirstTimeInStateActionsIfNeeded entered Election state - performing first actions");
    SendStartElectionToAllMembers(gmm);
    
    Vote(gmm);
    
    m_isFirstIterationInState = false;
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

void Electing::Vote(GMM &gmm)
{
  if(!m_isVoteCastForPeriod)
  {
    int idToVoteFor = 0;
    if(gmm.GetLowestQuorumConnectedId(idToVoteFor))
    {
      SendVote(idToVoteFor, gmm);
      if(idToVoteFor == gmm.GetMyId())
      {
        gmm.AddLeaderVote(idToVoteFor);
      }
      m_isVoteCastForPeriod = true;
    }
  }
}

void Electing::SendVote(unsigned int voteOnId,GMM &gmm)
{
  LeaderElectionMsg voteMsg(LeaderElectionMsg::MsgType::ElectionVote);
  voteMsg.SetViewNumber(gmm.GetMyViewNumber());
  voteMsg.SetSenderId(gmm.GetMyId());
  voteMsg.SetVoteDstId(voteOnId);
  LogMsg(LogPrioInfo, "Electing::SendVote Voting on %u view %u",voteMsg.GetIdOfVoteDst(),voteMsg.GetViewNumber());
  for(auto & pSender : m_senders )
  {
    if(!pSender->Send(voteMsg))
    {
      LogMsg(LogPrioCritical, "ERROR: Electing::SendVote Failed to send ElectionVote message");
    }
  }
}

void Electing::RestartElectionPeriodIfTmo(GMM &gmm)
{
  static const std::chrono::milliseconds electionPeriodTimeInMs(5000);
  const auto now = std::chrono::system_clock::now();
  const auto elapsed = now - m_startOfElectionPeriod;
  if(elapsed > electionPeriodTimeInMs)
  {
    const unsigned int largestViewSeenIncremented = gmm.GetLargestViewNumber() + 1;
    LogMsg(LogPrioInfo, "Electing::RestartElectionPeriodIfTmo  election period expired - restarting new view: %u", largestViewSeenIncremented);
    RestartElectionPeriod(largestViewSeenIncremented, gmm);
  }
}

void Electing::RestartElectionPeriod(const unsigned int newViewNumber,GMM &gmm)
{
  LogMsg(LogPrioInfo, "Electing::RestartElectionPeriod Restarting election period, view number %u",newViewNumber);
  m_startOfElectionPeriod = std::chrono::system_clock::now();
  gmm.ResetLeaderVoteCount();
  gmm.SetMyViewNumber(newViewNumber);
  
  SendStartElectionToAllMembers(gmm);
  m_isVoteCastForPeriod = false;
}

void Electing::Print() const
{
  LogMsg(LogPrioInfo, "State Electing");
}
