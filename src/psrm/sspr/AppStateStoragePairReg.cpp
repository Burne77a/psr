#include "AppStateStoragePairReg.h"
#include "Logger.h"
#include <errnoLib.h> 

AppStateStoragePairReg::AppStateStoragePairReg()
{
  
}

bool AppStateStoragePairReg::AddEntry(AppStateStoragePair &entryToAdd)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool isSuccessfullyAdded = false;
  auto existingEntry = m_appStatePairEntries.find(entryToAdd.GetAppId());
  if( existingEntry != m_appStatePairEntries.end())
  {
    LogMsg(LogPrioCritical,"ERROR: AppStateStoragePairReg::AddEntry with app id %u already exist",entryToAdd.GetAppId());
    existingEntry->second.Print();
  }
  else
  {
    m_appStatePairEntries.insert(std::make_pair(entryToAdd.GetAppId(), entryToAdd));
    isSuccessfullyAdded = true;
  }
  return isSuccessfullyAdded;
}

void AppStateStoragePairReg::RemoveEntry(const unsigned int appId)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto existingEntry = m_appStatePairEntries.find(appId);
  if( existingEntry != m_appStatePairEntries.end())
  {
    LogMsg(LogPrioInfo,"AppStateStoragePairReg::RemoveEntry with app id %u is removed",appId);
    m_appStatePairEntries.erase(existingEntry);
  }
  else
  {
    LogMsg(LogPrioWarning,"WARNING: AppStateStoragePairReg::RemoveEntry with app id %u is not found",appId);
  }
}

std::optional<AppStateStoragePair> AppStateStoragePairReg::GetEntry(const unsigned int appId) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto existingEntry = m_appStatePairEntries.find(appId);
  if( existingEntry != m_appStatePairEntries.end())
  {
    return existingEntry->second;
  }
  else
  {
    return std::nullopt;
  }
}

std::optional<unsigned int> AppStateStoragePairReg::FindLowestUsedStateStorage(unsigned int &usingAppsCount) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::unordered_map<unsigned int, unsigned int> storageUsageCounts; 
  std::optional<unsigned int> lowestStorageId{std::nullopt};
  
  usingAppsCount = std::numeric_limits<unsigned int>::max();
  
  for (const auto& pair : m_appStatePairEntries) 
  {
      unsigned int storageId = pair.second.GetStorageId();
      if (storageId != AppStateStoragePair::INVALID_VALUE) 
      {
          storageUsageCounts[storageId]++;
      }
  }

  for (const auto& countPair : storageUsageCounts) 
  {
      if (countPair.second < usingAppsCount) 
      {
          usingAppsCount = countPair.second;
          lowestStorageId = countPair.first;
      }
  }
  return lowestStorageId;
}

std::optional<unsigned int> AppStateStoragePairReg::FindLowestUsedStateStorageOnDifferentNode(unsigned int &usingAppsCount,const unsigned int nodeId) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  std::unordered_map<unsigned int, unsigned int> storageUsageCounts; 
  std::optional<unsigned int> lowestStorageId{std::nullopt};
  usingAppsCount = std::numeric_limits<unsigned int>::max();
  
  for (const auto& pair : m_appStatePairEntries) 
  {
      unsigned int storageId = pair.second.GetStorageId();
      if ((storageId != AppStateStoragePair::INVALID_VALUE) && (nodeId != pair.second.GetStorageNodeId())) 
      {
          storageUsageCounts[storageId]++;
      }
  }

  for (const auto& countPair : storageUsageCounts) 
  {
      if (countPair.second < usingAppsCount) 
      {
          usingAppsCount = countPair.second;
          lowestStorageId = countPair.first;
      }
  }
  return lowestStorageId;
}

void AppStateStoragePairReg::GetAllAppIdThatUseThisStorageId(const unsigned int storageId, std::vector<unsigned int>& appIds) const
{
  std::lock_guard<std::mutex> lock(m_mutex); 
  appIds.clear();
  for (const auto& pair : m_appStatePairEntries) 
  {
    if (pair.second.GetStorageId() == storageId) 
    {
      appIds.push_back(pair.second.GetAppId());
    }
  }
}

void AppStateStoragePairReg::Print() const
{
  LogMsg(LogPrioInfo,"--- >AppStateStoragePairReg< ---");
  LogMsg(LogPrioInfo,"Number of entries: %d", m_appStatePairEntries.size());
  for(auto &pEntry : m_appStatePairEntries)
  {
    pEntry.second.Print();
  }
  LogMsg(LogPrioInfo,"--- <AppStateStoragePairReg> ---");
}
