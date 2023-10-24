#include "LogEntry.h"
#include "Logger.h"
#include <errnoLib.h> 


std::unique_ptr<LogEntry> LogEntry::CreateEntry(const LogReplicationMsg msg)
{
  std::unique_ptr<LogEntry> pNewEntry = std::make_unique<LogEntry>(msg.GetOpNumber(),msg.GetViewNumber(),msg.GetReqId());
  if(!pNewEntry)
  {
    LogMsg(LogPrioCritical, "ERROR LogEntry::CreateEntry failed to LogEntry. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
  return pNewEntry;
}

LogEntry::LogEntry(const unsigned int opNumber, const unsigned int viewNumber, const unsigned int rid) :
m_opNumber{opNumber},m_viewNumber{viewNumber}, m_rid{rid}
{
 
}


void LogEntry::Print() const
{
  LogMsg(LogPrioInfo,"LogEntry: Op %u View %u Rid %u %s %s",m_opNumber,m_viewNumber,m_rid, m_isCommited ? "COMMITED" : "NOT COMMITED", m_isUpcallDone ? "UPCALL MADE" : "UPCALL TBD");
}

bool LogEntry::IsValidNextOpAndViewNumber(const unsigned int opNumber, const unsigned int viewNumber)
{
  bool isValidNextEntry = false;
  if(opNumber == m_opNumber +1)
  {
    if(viewNumber >= m_viewNumber)
    {
      isValidNextEntry = true;
    }
  }
  return isValidNextEntry;
}
