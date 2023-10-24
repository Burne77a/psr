#include "ReplicatedLog.h"
#include "Logger.h"

ReplicatedLog::ReplicatedLog()
{
 
} 

bool ReplicatedLog::ArePreviousEntriesInLog(const unsigned int opNumber, const unsigned int viewNumber)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool arePreviousEntriesInLog = false;
  auto &pEntry = GetLastValidEntry();
 
  if(pEntry)
  {
    if(pEntry->IsValidNextOpAndViewNumber(opNumber,viewNumber))
    {
      arePreviousEntriesInLog = true;
    }
  }
  
  return arePreviousEntriesInLog;
}

bool ReplicatedLog::IsEntryAlreadyExisting(const unsigned int opNumber, const unsigned int viewNumber)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool isEntryAlreadyExisting = false; 
  auto &pEntry = GetEntryWithOpNumber(opNumber);
  if(pEntry)
  {
    isEntryAlreadyExisting = (pEntry->GetViewNumber() == viewNumber);
  }
  
  return isEntryAlreadyExisting;
}

bool ReplicatedLog::AddEntryToLogIfNotAlreadyIn(const LogReplicationMsg &msgToMakeEntryFrom)
{
  bool isSuccessfullyHandled = false;
  if(!ArePreviousEntriesInLog(msgToMakeEntryFrom))
  {
    LogMsg(LogPrioError,"ReplicatedLog::AddEntryToLog() trying to add entry without the preceding entries in place - aborting add");
    msgToMakeEntryFrom.Print();
  }
  else
  {
    if(!IsEntryAlreadyExisting(msgToMakeEntryFrom))
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      if(AddOrOverwriteDependingOnView(msgToMakeEntryFrom))
      {
        LogMsg(LogPrioInfo,"ReplicatedLog::AddEntryToLog() Successfully added or replaced entry");
        isSuccessfullyHandled = true;
      }
      else
      {
        LogMsg(LogPrioCritical,"ERROR: ReplicatedLog::AddEntryToLog() Failed to add or replace entry");
      }
    }
    else
    {
      LogMsg(LogPrioInfo,"ReplicatedLog::AddEntryToLog() entry already existing");
      isSuccessfullyHandled = true;
      
    }
  }
  return isSuccessfullyHandled;
}

bool ReplicatedLog::AddOrOverwriteDependingOnView(const LogReplicationMsg &msg)
{
  bool isHandledOK = false;
  auto & pExistingEntry = GetEntryWithOpNumber(msg.GetOpNumber());
  if(pExistingEntry)
  {
    if(pExistingEntry->GetViewNumber() < msg.GetViewNumber())
    {
      if(!pExistingEntry->IsCommited())
      {
        pExistingEntry = LogEntry::CreateEntry(msg);
        if(pExistingEntry)
        {
          LogMsg(LogPrioInfo,"ReplicatedLog::AddOrOverwriteDependingOnView() - Replaced uncommitted entry");
          isHandledOK = true;
        }
        else
        {
          LogMsg(LogPrioCritical,"ERROR: ReplicatedLog::AddEntryToLog() Failed to create entry for replacement.");
        }
      }
      else
      {
        LogMsg(LogPrioInfo,"ReplicatedLog::AddOrOverwriteDependingOnView() - Received the same, committed message, again");
        isHandledOK = true;
      }
    }
    else if(pExistingEntry->GetViewNumber() > msg.GetViewNumber())
    {
      LogMsg(LogPrioCritical,"ERROR: ReplicatedLog::AddEntryToLog() Recived a view number lower than what is committed");
      pExistingEntry->Print();
      msg.Print();
    }
  }
  else
  {
    std::unique_ptr<LogEntry> pEntry{LogEntry::CreateEntry(msg)};
    if(pEntry)
    {
      LogMsg(LogPrioInfo,"ReplicatedLog::AddEntryToLog() entry added");
      pEntry->Print();
      m_logEntries.push_back(std::move(pEntry));
      isHandledOK = true;
     
    }
    else
    {
      LogMsg(LogPrioCritical,"ERROR: ReplicatedLog::AddEntryToLog() Failed to create entry.");
    }
  }
  return isHandledOK;
}

std::unique_ptr<LogEntry> & ReplicatedLog::GetEntryWithOpNumber(const unsigned int opNumber)
{
  static std::unique_ptr<LogEntry> noValidEntry{nullptr};
  for(auto &pEntry : m_logEntries)
  {
    if(pEntry)
    {
      if(pEntry->GetOpNumber() == opNumber)
      {
        return pEntry;
      }
    }
  }
  return noValidEntry;
}


std::unique_ptr<LogEntry> & ReplicatedLog::GetLastValidEntry()
{
  static std::unique_ptr<LogEntry> noValidEntry{nullptr};
  const int numberOfEntries = m_logEntries.size();
  for(int i = numberOfEntries; i > 0; i-- )
  {
    auto &pEntry = m_logEntries[i];
    if(pEntry)
    {
      return m_logEntries[i];
    }
  }
  return noValidEntry;
}

void ReplicatedLog::Print() const
{
  LogMsg(LogPrioInfo,"--- ReplicatedLog ---");
  LogMsg(LogPrioInfo,"Number of entries: %d", m_logEntries.size());
  for(auto &pEntries : m_logEntries)
  {
    pEntries->Print();
  }
  LogMsg(LogPrioInfo,"--- ----- ---");
}
