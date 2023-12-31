#include "StorageReg.h"
#include "Logger.h"
#include <errnoLib.h> 

StorageReg::StorageReg()
{
  
}

bool StorageReg::AddEntry(StorageInfo &entryToAdd)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool isSuccessfullyAdded = false;
  auto existingEntry = m_storageInfoEntries.find(entryToAdd.GetId());
  if( existingEntry != m_storageInfoEntries.end())
  {
    LogMsg(LogPrioCritical,"ERROR: StorageReg::AddEntry with id %u already exist",entryToAdd.GetId());
    existingEntry->second.Print();
  }
  else
  {
    m_storageInfoEntries.insert(std::make_pair(entryToAdd.GetId(), entryToAdd));
    isSuccessfullyAdded = true;
  }
  return isSuccessfullyAdded;
}

void StorageReg::RemoveEntry(unsigned int storageId)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto existingEntry = m_storageInfoEntries.find(storageId);
  if( existingEntry != m_storageInfoEntries.end())
  {
    LogMsg(LogPrioInfo,"StorageReg::RemoveEntry with id %u is removed",storageId);
    m_storageInfoEntries.erase(existingEntry);
  }
  else
  {
    LogMsg(LogPrioWarning,"WARNING: StorageReg::RemoveEntry with id %u is not found",storageId);
  }
}

std::optional<StorageInfo> StorageReg::GetStorageInfo(const unsigned int storageId) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto existingEntry = m_storageInfoEntries.find(storageId);
  if( existingEntry != m_storageInfoEntries.end())
  {
    return existingEntry->second;
  }
  else
  {
    return std::nullopt;
  }
}

void StorageReg::GetAllStorageIds(std::vector<unsigned int> &storageIds) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  storageIds.clear();
  for(auto &entry : m_storageInfoEntries)
  {
    storageIds.push_back(entry.second.GetId());
  }
}

void StorageReg::Print() const
{
  LogMsg(LogPrioInfo,"--- >StorageReg< ---");
  LogMsg(LogPrioInfo,"Number of entries: %d", m_storageInfoEntries.size());
  for(auto &entry : m_storageInfoEntries)
  {
    entry.second.Print();
  }
  LogMsg(LogPrioInfo,"--- <StorageReg> ---");
}
