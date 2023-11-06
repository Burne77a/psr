#include "AppReg.h"
#include "Logger.h"
#include <errnoLib.h> 

AppReg::AppReg()
{
  
}

bool AppReg::AddEntry(AppInfo &entryToAdd)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool isSuccessfullyAdded = false;
  auto existingEntry = m_appInfoEntries.find(entryToAdd.GetId());
  if( existingEntry != m_appInfoEntries.end())
  {
    LogMsg(LogPrioCritical,"ERROR: AppReg::AddEntry with id %u already exist",entryToAdd.GetId());
    existingEntry->second.Print();
  }
  else
  {
    m_appInfoEntries.insert(std::make_pair(entryToAdd.GetId(), entryToAdd));
    isSuccessfullyAdded = true;
  }
  return isSuccessfullyAdded;
}

void AppReg::RemoveEntry(unsigned int appId)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  auto existingEntry = m_appInfoEntries.find(appId);
  if( existingEntry != m_appInfoEntries.end())
  {
    LogMsg(LogPrioInfo,"AppReg::RemoveEntry with id %u is removed",appId);
    m_appInfoEntries.erase(existingEntry);
  }
  else
  {
    LogMsg(LogPrioWarning,"WARNING: AppReg::RemoveEntry with id %u is not found",appId);
  }
}

void AppReg::Print() const
{
  LogMsg(LogPrioInfo,"--- AppReg ---");
  LogMsg(LogPrioInfo,"Number of entries: %d", m_appInfoEntries.size());
  for(auto &pEntry : m_appInfoEntries)
  {
    pEntry.second.Print();
  }
  LogMsg(LogPrioInfo,"--- ----- ---");
}
