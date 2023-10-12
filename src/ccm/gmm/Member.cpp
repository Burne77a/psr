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

void Member::SetConnectionPerception(const std::bitset<MAX_MEMBERS> &connectionPerception)
{
  m_connections = connectionPerception;
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

void Member::UpdateHeartbeat() 
{
  m_lastHeartbeat = std::chrono::system_clock::now();
}

std::chrono::system_clock::time_point Member::GetLastHeartbeat() const 
{
  return m_lastHeartbeat;
}

bool Member::HasHeartbeatExceeded(const std::chrono::milliseconds& duration) const 
{
  auto now = std::chrono::system_clock::now();
  auto elapsed = now - m_lastHeartbeat;
  return elapsed > duration;
}


