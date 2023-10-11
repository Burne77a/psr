#include "GMM.h"

void GMM::AddMember(const int id, const std::string& name, const std::string& ip) 
{
  m_members.insert(std::make_pair(id, Member(id, name, ip)));
}

Member* GMM::GetMember(const int id) 
{
  if (m_members.find(id) != m_members.end()) 
  {
    return &m_members.at(id);
  }
  return nullptr;
}

void GMM::EstablishConnection(const int id1, const int id2) 
{
  if (m_members.find(id1) != m_members.end() && m_members.find(id2) != m_members.end()) 
  {
    m_members.at(id1).AddConnection(id2);
    m_members.at(id2).AddConnection(id1);
  }
}

bool GMM::IsQuorumConnected(const int id)
{
  if (m_members.find(id) == m_members.end()) 
  {
    return false;
  }

  const int totalMembers = m_members.size();
  const int requiredConnections = (totalMembers / 2) + (totalMembers % 2);
  return (m_members.at(id).ConnectionCount() + 1) >= requiredConnections;
  return false;
}

void GMM::UpdateMemberHeartbeat(const int id) 
{
  if (auto member = GetMember(id)) 
  {
    member->UpdateHeartbeat();
  }
}

std::chrono::system_clock::time_point GMM::GetMemberLastHeartbeat(const int id) 
{
  if (auto member = GetMember(id)) 
  {
      return member->GetLastHeartbeat();
  }
  return std::chrono::system_clock::time_point(); // Return epoch time if member is not found
}

bool GMM::HasMemberHeartbeatExceeded(const int id, const std::chrono::milliseconds& duration) 
{
  if (auto member = GetMember(id)) 
  {
      return member->HasHeartbeatExceeded(duration);
  }
  return true; // Consider it as exceeded if member is not found
}

void GMM::ForEachMember(const std::function<void(const int, Member&)>& func) 
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for (auto& pair : m_members) 
  {
    func(pair.first, pair.second);
  }
}
