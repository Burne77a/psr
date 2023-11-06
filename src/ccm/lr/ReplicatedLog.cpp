#include "ReplicatedLog.h"
#include "Logger.h"

ReplicatedLog::ReplicatedLog(UpcallReplicatedLogCallbackType upcallCb) : m_upcallCb{upcallCb}
{
 
} 

bool ReplicatedLog::ArePreviousEntriesInLog(const unsigned int opNumber, const unsigned int viewNumber)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool arePreviousEntriesInLog = false;
  auto &pEntry = GetLastValidEntry();
  
  if(opNumber == 1)
  {
    arePreviousEntriesInLog = true;
  }
 
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

bool ReplicatedLog::ArePreviousEntriesCommitted(const unsigned int opNumber)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool allPreviousAreCommitted = true;
  for(auto &pEntry : m_logEntries)
  {
    if(pEntry)
    {
      if(pEntry->GetOpNumber() >= opNumber)
      {
        break;
      }
      else
      {
        if(!pEntry->IsCommited())
        {
          allPreviousAreCommitted = false;
          break;
        }
      }
    }
  }
  return allPreviousAreCommitted;
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

bool ReplicatedLog::CommitEntryIfPresent(const LogReplicationMsg &msgToCommitCorespondingEntryFor)
{
  bool isSuccessfullyCommitted = false; 
  if(IsEntryAlreadyExisting(msgToCommitCorespondingEntryFor))
  {
    if(ArePreviousEntriesCommitted(msgToCommitCorespondingEntryFor.GetOpNumber()))
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      auto & pEntry = GetEntryWithOpNumber(msgToCommitCorespondingEntryFor.GetOpNumber());
      if(pEntry)
      {
        pEntry->SetCommitted();
        isSuccessfullyCommitted = true;
        LogMsg(LogPrioInfo,"ReplicatedLog::CommitEntryIfPresent() - entry committed");
        pEntry->Print();
      }
      else
      {
        LogMsg(LogPrioCritical,"ReplicatedLog::CommitEntryIfPresent() - failed to find entry");
        msgToCommitCorespondingEntryFor.Print();
      }
    }
    else
    {
      LogMsg(LogPrioWarning,"ReplicatedLog::CommitEntryIfPresent() - all previous entries are not committed %u",msgToCommitCorespondingEntryFor.GetOpNumber());
      msgToCommitCorespondingEntryFor.Print();
    }
  }
  else
  {
    LogMsg(LogPrioCritical,"ReplicatedLog::CommitEntryIfPresent() - trying to commit entry that does not exist in log.");
    msgToCommitCorespondingEntryFor.Print();
  }
  return isSuccessfullyCommitted;
}

void ReplicatedLog::PerformUpcalls(const bool isForce)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for(auto &pEntry : m_logEntries)
  {
    if(pEntry)
    {
      if(pEntry->IsCommited())
      {
        if(!pEntry->IsUpcallDone() || isForce)
        {
          if(m_upcallCb)
          {
            m_upcallCb(pEntry->GetClientMsg());
            LogMsg(LogPrioInfo,"ReplicatedLog::PerformUpcalls() - Upcall performed. OpNumber %u",pEntry->GetOpNumber());
          }
          else
          {
            LogMsg(LogPrioWarning,"ReplicatedLog::PerformUpcalls() - No upcall callback found. OpNumber %u",pEntry->GetOpNumber());
          }
          pEntry->SetUpCallDone();
        }
      }
    }
  }
}

unsigned int ReplicatedLog::GetLatestEntryOpNumber()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  unsigned int opNumberToReturn = 0U;
  auto & pExistingEntry = GetLastValidEntry();
  if(pExistingEntry)
  {
    opNumberToReturn = pExistingEntry->GetOpNumber();
  }
  return opNumberToReturn;
}


bool ReplicatedLog::GetLogEntriesAsSyncMsgVector(const int myId, std::vector<std::shared_ptr<SyncMsg>> &vectorToPopulate)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool isSuccessfullyPopulated = true;
  const unsigned int entries = m_logEntries.size();
  unsigned int currentEntry = 0;
  for(auto &pEntry : m_logEntries)
  {
    if(pEntry)
    {
      std::shared_ptr<SyncMsg> pSyncMsg = SyncMsg::CreateSyncMsgFromLogEntry(myId,entries,*pEntry,currentEntry);
      if(pSyncMsg)
      {
        vectorToPopulate.push_back(pSyncMsg);
        currentEntry++;
      }
      else
      {
        LogMsg(LogPrioCritical,"ERROR: ReplicatedLog::GetLogEntriesAsSyncMsgVector() Failed to create SyncMsg from entry");
        isSuccessfullyPopulated = false;
      }
    }
  }
  return isSuccessfullyPopulated;
}

void ReplicatedLog::PopulateFromVector(std::vector<std::shared_ptr<SyncMsg>> &vectorToPopulateFrom)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for(unsigned int i = 0; i < vectorToPopulateFrom.size(); i++)
  {
    auto pSyncMsg = vectorToPopulateFrom[i];
    if(pSyncMsg)
    {
      auto pNewEntry = LogEntry::CreateEntry(*pSyncMsg);
      if(pNewEntry)
      {
        auto & pExistingEntry = GetEntryWithOpNumber(pNewEntry->GetOpNumber());
        if(!pExistingEntry)
        {
          m_logEntries.push_back(std::move(pNewEntry));
        }
      }
      else
      {
        LogMsg(LogPrioCritical,"ERROR: ReplicatedLog::PopulateFromVector()failed to create LogEntry from sync %u",i);
      }
    }
    else
    {
      LogMsg(LogPrioCritical,"ERROR: ReplicatedLog::PopulateFromVector() invalid entry in vector %u",i);
    }
    
  }
}

void ReplicatedLog::CommittAllEarlierEntries(const unsigned int opNumber)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  for(auto &pEntry : m_logEntries)
  {
    if(pEntry)
    {
      if(pEntry->GetOpNumber() <= opNumber)
      {
        pEntry->SetCommitted();
      }
    }
  }
}

unsigned int ReplicatedLog::GetHighestCommittedOpNumber()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  unsigned int highestCommittedOpNumber = 0U;
  for(auto &pEntry : m_logEntries)
  {
    if(pEntry)
    {
      if(pEntry->GetOpNumber() > highestCommittedOpNumber)
      {
        if(pEntry->IsCommited())
        {
          highestCommittedOpNumber = pEntry->GetOpNumber();
        }
      }
    }
  }
  return highestCommittedOpNumber;
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
  for(int i = numberOfEntries - 1; i >= 0; i-- )
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
  LogMsg(LogPrioInfo,"--- >ReplicatedLog< ---");
  LogMsg(LogPrioInfo,"Number of entries: %d", m_logEntries.size());
  for(auto &pEntries : m_logEntries)
  {
    pEntries->Print();
  }
  LogMsg(LogPrioInfo,"--- <ReplicatedLog> ---");
}
