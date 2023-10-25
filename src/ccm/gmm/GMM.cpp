#include "GMM.h"
#include <algorithm>

GMM::GMM(const int myId) : m_myId(myId)
{
  
}

void GMM::AddMember(const int id, const std::string& name, const std::string& ip) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_members.insert(std::make_pair(id, Member(id, name, ip)));
}

void GMM::EstablishConnection(const int id1, const int id2) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_members.find(id1) != m_members.end() && m_members.find(id2) != m_members.end()) 
  {
    m_members.at(id1).AddConnection(id2);
    m_members.at(id2).AddConnection(id1);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::EstablishConnection member(s) not found %d %d",id1, id2);
  }
}

bool GMM::IsQuorumConnected(const int id)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return IsQuorumConnectedNoLock(id);
}


bool GMM::GetLowestQuorumConnectedId(int &id)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool isQcFound = false;
  id = 0;
  for (auto& pair : m_members) 
  {
    const int currentMemberId = pair.second.GetID();
   
    if(IsQuorumConnectedNoLock(currentMemberId))
    {
      if(id==0 || currentMemberId < id)
      {
        id = currentMemberId;
        isQcFound = true;
      }
     }
  }
  return isQcFound;
}

bool GMM::IsAnyMemberQuorumConnected()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto& pair : m_members) 
  {
    if(IsQuorumConnectedNoLock(pair.second.GetID()))
    {
      return true;
    }
  }
  return false;
}



void GMM::UpdateMemberHeartbeat(const HeartbeatCCM & hb) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (auto pMember = GetMember(hb.GetSenderId())) 
  {
    pMember->UpdateHeartbeat(hb.GetLeaderId());
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::UpdateMemberHeartbeat member not found %d",hb.GetSenderId());
  }
}

std::chrono::system_clock::time_point GMM::GetMemberLastHeartbeat(const int id) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (auto pMember = GetMember(id)) 
  {
      return pMember->GetLastHeartbeat();
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::GetMemberLastHeartbeat member not found %d",id);
    return std::chrono::system_clock::time_point(); // Return epoch time if member is not found
  }
  
}

bool GMM::HasMemberHeartbeatExceeded(const int id, const std::chrono::milliseconds& duration) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (auto pMember = GetMember(id)) 
  {
      return pMember->HasHeartbeatExceeded(duration);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::HasMemberHeartbeatExceeded member not found %d",id);
    return true; // Consider it as exceeded if member is not found
  } 
}

void GMM::UpdateConnectionStatusForMySelf(const std::chrono::milliseconds& duration)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMySelf = GetMember(m_myId);
  if(pMySelf)
  {
    for (auto& pair : m_members) 
    {
      if(pair.second.GetID() != m_myId)
      {
        if(pair.second.HasHeartbeatExceeded(duration))
        {
          pMySelf->RemoveConnection(pair.second.GetID());
          pair.second.ClearLeaderId();
          pair.second.ResetConnectionPerception();
        }
        else
        {
          pMySelf->AddConnection(pair.second.GetID());
        }
      }
    }
  }
}

void GMM::SetMySelfToLeader()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMySelf = GetMember(m_myId);
  if(pMySelf)
  {
    pMySelf->SetLeaderId(m_myId);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::SetMySelfToLeader member not found %d",m_myId);
  }
}

void GMM::RemoveMySelfAsLeader()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMySelf = GetMember(m_myId);
  if(pMySelf)
  {
    pMySelf->ClearLeaderId();
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::RemoveMySelfAsLeader member not found %d",m_myId);  
  }
}

int GMM::GetLeaderId(void)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMySelf = GetMember(m_myId);
  int leaderId = INVALID_LEADER_ID; 
 
  for (auto& pair : m_members) 
  {
    leaderId = pair.second.GetLeaderId();
    if((leaderId != INVALID_LEADER_ID) && (leaderId == pair.second.GetID()))
    {
      break;
    }
  }
  return leaderId;
}

std::string GMM::GetLeaderIp()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMySelf = GetMember(m_myId);
  int leaderId = INVALID_LEADER_ID; 
  std::string leaderIpAddr{"None"};
  for (auto& pair : m_members) 
  {
    leaderId = pair.second.GetLeaderId();
    if((leaderId != INVALID_LEADER_ID) && (leaderId == pair.second.GetID()))
    {
      leaderIpAddr = pair.second.GetIP();
      break;
    }
  }
  return leaderIpAddr;
}

void GMM::ResetLeaderVoteCount()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto& pair : m_members) 
  {
    pair.second.ResetVoteCnt();
  }
}

void GMM::AddLeaderVote(const int id)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMember = GetMember(id);
  if(pMember)
  {
    pMember->IncrementVoteCnt();
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::AddLeaderVote member not found %d",id);
  }
  
}

void GMM::SetViewNumber(const int id, const unsigned int viewNumber)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMember = GetMember(id);
  if(pMember)
  {
    pMember->SetViewNumber(viewNumber);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::SetViewNumber member not found %d",id);
  }
}

const unsigned int GMM::GetViewNumber(const int id) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMember = GetMember(id);
  unsigned int viewNumberToReturn = 0U;
  if(pMember)
  {
    viewNumberToReturn = pMember->GetViewNumber();
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::GetViewNumber member not found %d",id);
  }
  return viewNumberToReturn;
}

void GMM::SetMyViewNumber(const unsigned int viewNumber)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMyMember = GetMember(m_myId);
  if(pMyMember)
  {
    pMyMember->SetViewNumber(viewNumber);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::SetMyViewNumber member not found %d",m_myId);
  }
}

const unsigned int GMM::GetMyViewNumber()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  unsigned int viewNumberToReturn = 0U;
  auto pMyMember = GetMember(m_myId);
  if(pMyMember)
  {
    viewNumberToReturn = pMyMember->GetViewNumber();
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::GetMyViewNumber member not found %d",m_myId);
  }
  return viewNumberToReturn;
}

const unsigned int GMM::GetLargestViewNumber()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  unsigned int maxViewNumberToReturn = 0U;
  for (auto& pair : m_members) 
  {
    maxViewNumberToReturn = std::max<unsigned int>(pair.second.GetViewNumber(),maxViewNumberToReturn);
  }
  return maxViewNumberToReturn;
}

bool GMM::IsVoteCntForMySelfLargerThanMajority()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto pMyMember = GetMember(m_myId);
  bool isAMajorityReached = false;
  if(pMyMember)
  {
    const unsigned int votes = pMyMember->GetLeaderVoteCnt();
    const unsigned int total = m_members.size();
    const unsigned int halfRoundedUp = (total/2) +1 ;
    isAMajorityReached =  (votes >= halfRoundedUp);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::IsVoteCntForMySelfLargerThanMajority member not found %d",m_myId);
  }
  return isAMajorityReached;
}


void GMM::ForEachMember(const std::function<void(const int, Member&)>& func) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto& pair : m_members) 
  {
    func(pair.first, pair.second);
  }
}

void GMM::ForIdMember(const int id, const std::function<void(const int, Member&)>& func)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (auto pMember = GetMember(id)) 
  {
    func(m_myId,*pMember);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::ForIdMember member not found %d",m_myId);
  }
}

void GMM::ForMyMember(const std::function<void(const int, Member&)>& func)
{
  ForIdMember(m_myId,func);
}

Member* GMM::GetMember(const int id) 
{
  if (m_members.find(id) != m_members.end()) 
  {
    return &m_members.at(id);
  }
  return nullptr;
}


bool GMM::IsQuorumConnectedNoLock(const int id) const
{
  if (m_members.find(id) == m_members.end()) 
  {
   LogMsg(LogPrioCritical,"ERROR: GMM::IsQuorumConnected member not found %d",id);
   return false;
  }
  
  const int totalMembers = m_members.size();
  const int requiredConnections = (totalMembers / 2) +1;
  return (m_members.at(id).ConnectionCount() + 1) >= requiredConnections;
}

void GMM::Print() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  LogMsg(LogPrioInfo,"--- GMM members ---");
  
  LogMsg(LogPrioInfo,"My Member: ");
  if (m_members.find(m_myId) != m_members.end()) 
  {
    const auto pMyMember = &m_members.at(m_myId);
    if(pMyMember)
    {
      pMyMember->Print();
      LogMsg(LogPrioInfo,"QC: %s ",IsQuorumConnectedNoLock(m_myId) ? "Yes": "No");
    }
  }
  LogMsg(LogPrioInfo,"The other members: ");
  for (auto& pair : m_members) 
  {
    if(m_myId != pair.second.GetID())
    {
      pair.second.Print();
      LogMsg(LogPrioInfo,"QC: %s ",IsQuorumConnectedNoLock(pair.second.GetID()) ? "Yes": "No");
    }
  }
  LogMsg(LogPrioInfo,"-------------");
}


