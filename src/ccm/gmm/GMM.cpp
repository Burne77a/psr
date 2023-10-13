#include "GMM.h"


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
  if (m_members.find(id) == m_members.end()) 
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::IsQuorumConnected member not found %d",id);
    return false;
  }

  const int totalMembers = m_members.size();
  const int requiredConnections = (totalMembers / 2) + (totalMembers % 2);
  return (m_members.at(id).ConnectionCount() + 1) >= requiredConnections;
}

void GMM::UpdateMemberHeartbeat(const int id) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (auto pMember = GetMember(id)) 
  {
    pMember->UpdateHeartbeat();
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: GMM::UpdateMemberHeartbeat member not found %d",id);
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
        }
        else
        {
          pMySelf->AddConnection(pair.second.GetID());
        }
      }
    }
  }
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
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_members.find(id) != m_members.end()) 
  {
    return &m_members.at(id);
  }
  return nullptr;
}
