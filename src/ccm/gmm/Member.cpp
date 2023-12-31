#include "Member.h"
#include "Logger.h"
#include <string>

Member::Member(const int id, const std::string& name, const std::string& ip)
    : m_id(id), m_name(name), m_ip(ip) {}

void Member::AddConnection(const int memberId) 
{
  if(memberId >= 0 && memberId <= MAX_MEMBERS)
  {
    if(!m_connections.test(memberId))
    {
      LogMsg(LogPrioInfo," Member connection changed. Connection added: %d --> %d",m_id,memberId);
    }
    m_connections.set(memberId);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: Member::AddConnection invalid member id %d",memberId);
  }
}

void Member::RemoveConnection(const int memberId)
{
  if(memberId >= 0 && memberId <= MAX_MEMBERS)
  {
    if(m_connections.test(memberId))
    {
      LogMsg(LogPrioInfo," Member connection changed. Connection removed: %d -/> %d",m_id,memberId);
    }
    m_connections.reset(memberId);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: Member::RemoveConnection invalid member id %d",memberId);
  }
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

void Member::ResetConnectionPerception()
{
  m_connections = 0U;
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

void Member::UpdateHeartbeat(const int leaderId) 
{
  m_lastHeartbeat = std::chrono::system_clock::now();
  m_leaderId = leaderId;
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

void Member::SetPrepareOkRcvdIfMatchPending(const unsigned int viewNumber, const unsigned int opNumber)
{
  if((viewNumber != INVALID_REQ_ID) && (opNumber != INVALID_REQ_ID))
  {
    if((viewNumber == m_pendingPrepeareViewNumber) && (opNumber == m_pedingPrepareOpNumber))
    {
      m_isValidPrepareOkRcvd = true;
    }
  }
}

void Member::Print() const
{
  std::string bitsAsStr("");
  for(int i = 0; i < m_connections.size(); i++)
  {
    if(m_connections.test(i))
    {
      bitsAsStr += "1";
    }
    else
    {
      bitsAsStr += "0";
    }
  }
  LogMsg(LogPrioInfo,"Member %d name %s ip %s ",m_id,m_name.c_str(),m_ip.c_str());
  LogMsg(LogPrioInfo,"Connections %s",bitsAsStr.c_str());
  LogMsg(LogPrioInfo,"PendingPrepareId View: %u op: %u Valid PrepareOK rcvd: %s", m_pendingPrepeareViewNumber,m_pedingPrepareOpNumber, m_isValidPrepareOkRcvd? "Yes": "No");
  if(m_leaderId == INVALID_LEADER_ID)
  {
    LogMsg(LogPrioInfo,"No leader. Vote count: %d",m_leaderVoteCnt);
  }
  else
  {
    if(m_leaderId == m_id)
    {
      LogMsg(LogPrioInfo,"I'm leader!");
    }
    else
    {
      LogMsg(LogPrioInfo,"Leader ID %d",m_leaderId);
    }
  }
  LogMsg(LogPrioInfo,"View number %u OpNumber %u Committed OpNumber: %u",m_viewNumber,m_operationNumber,m_committedOpNumber);
}


