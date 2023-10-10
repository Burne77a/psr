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
