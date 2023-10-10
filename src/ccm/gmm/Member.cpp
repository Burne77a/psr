#include "Member.h"

Member::Member(const int id, const std::string& name, const std::string& ip)
    : m_id(id), m_name(name), m_ip(ip) {}

void Member::AddConnection(int memberId) 
{
    m_connections.set(memberId);
}

int Member::ConnectionCount() const 
{
  return m_connections.count();
}

std::bitset<MAX_MEMBERS> Member::GetConnections() const 
{
  return m_connections;
}

int Member::GetID() const 
{
  return m_id;
}

const std::string& Member::GetName() const 
{
  return m_name;
}

const std::string& Member::GetIP() const 
{
  return m_ip;
}
