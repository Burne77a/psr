#include "ASSP.h"
#include "Logger.h"
#include <functional>
#include <optional>
#include <errnoLib.h>

std::unique_ptr<ASSP> ASSP::CreateASSP(std::shared_ptr<ICCM>& pIccm,AIR &air,SR &sr,SSPR &sspr)
{
  std::unique_ptr<ASSP> pAssp{nullptr};
  
  pAssp = std::make_unique<ASSP>(pIccm,air,sr,sspr);
  if(!pAssp)
  {
    LogMsg(LogPrioCritical, "ERROR: ASSP::CreateASSP failed to create ASSP. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  air.RegisterAppAddedCb(std::bind(&ASSP::ApplicationAdded,&*pAssp, std::placeholders::_1));
  air.RegisterAppRemovedCb(std::bind(&ASSP::ApplicationRemoved,&*pAssp, std::placeholders::_1));
  
  sr.RegisterStorageAddedCb(std::bind(&ASSP::StorageAdded,&*pAssp, std::placeholders::_1));
  sr.RegisterStorageRemovedCb(std::bind(&ASSP::StorageRemoved,&*pAssp, std::placeholders::_1));
  
  
  return pAssp;
}

ASSP::ASSP(std::shared_ptr<ICCM>& pIccm,const AIR &air, const SR &sr,SSPR &sspr) : m_pIccm{pIccm},m_air{air}, m_sr{sr},m_sspr{sspr}
{
  
}

void ASSP::CheckForUnPairedAppAndTryToPair()
{
  std::vector<unsigned int> allAppIdsKnown;
  m_air.GetAllAppIds(allAppIdsKnown);
  for(auto appId : allAppIdsKnown)
  {
    PairAppWithStorageIfPossible(appId);
  }
}

bool ASSP::PairWithUnUsedIfItExist(const unsigned int appId)
{
  bool isSuccessfullyPairedWithUnusedStateStorage = false;
  std::vector<unsigned int> allKnownStorageIds;
  m_sr.GetAllStorageIds(allKnownStorageIds);
  for(auto storageId : allKnownStorageIds)
  {
    std::vector<unsigned int> appIdsThatUseThisStorageId;
    m_sspr.GetAllAppIdThatUseThisStorageId(storageId, appIdsThatUseThisStorageId);
    if(appIdsThatUseThisStorageId.empty())
    {
      auto storageInfoOrNothing = m_sr.GetStorageInfo(storageId);
      if(storageInfoOrNothing.has_value())
      {
        m_sspr.PostStoragePairForReplication(AppStateStoragePair{appId,storageId,storageInfoOrNothing.value().GetIpAddr()});
        LogMsg(LogPrioInfo,"ASSP::PairWithUnUsedIfItExist posted pairing of app %u and storage %u (%s)",appId,storageId,storageInfoOrNothing.value().GetIpAddr().c_str());
        isSuccessfullyPairedWithUnusedStateStorage = true;
      }
      else
      {
        LogMsg(LogPrioCritical,"ERROR: ASSP::PairWithUnUsedIfItExist failed to get storage info for ID that should exist %u and storage %u",appId,storageId);
      }
    }
        
  }
  return isSuccessfullyPairedWithUnusedStateStorage;
}

bool ASSP::PairWithUsed(const unsigned int appId)
{
  unsigned int useCnt = 0U;
  bool isMatchMade = false;
  
  auto storageIdOrNothing = m_sspr.FindLowestUsedStateStorage(useCnt);
  if(storageIdOrNothing.has_value())
  {
    unsigned int storageId = storageIdOrNothing.value();
    if(useCnt < STORAGE_USE_CNT_LIMIT)
    {
      //Do the pairing...
      auto storageInfoOrNothing = m_sr.GetStorageInfo(storageId);
      if(storageInfoOrNothing.has_value())
      {
        m_sspr.PostStoragePairForReplication(AppStateStoragePair{appId,storageId,storageInfoOrNothing.value().GetIpAddr()});
        LogMsg(LogPrioInfo,"ASSP::PairWithUsed posted pairing of app %u and storage %u (%s)",appId,storageId,storageInfoOrNothing.value().GetIpAddr().c_str());
        isMatchMade = true;
      }
      else
      {
        LogMsg(LogPrioError, "WARNING: ASSP::PairWithUsed Not able to find storage with id %u",storageId);
      }
    }
  }
  if(isMatchMade)
  {
    LogMsg(LogPrioWarning, "WARNING: ASSP::PairWithUsed Not able to find storage for app %u",appId);
  }
  return isMatchMade;
}

void ASSP::PairAppWithStorageIfPossible(const unsigned int appId)
{
  if(PairWithUnUsedIfItExist(appId))
  {
    return;
  }
  
  if(PairWithUsed(appId))
  {
    return;
  }
  
  LogMsg(LogPrioWarning, "WARNING: ASSP::PairAppWithStorageIfPossible Not able to find storage for app %u",appId);
}

void ASSP::RemoveAppStoragePair(const unsigned int appId)
{
  m_sspr.PostStoragePairForReplication(AppStateStoragePair{appId,AppStateStoragePair::INVALID_VALUE,std::string{"NONE"}});
}

void ASSP::ApplicationRemoved(const unsigned int appId)
{
  auto entryOrNothing = m_sspr.GetEntry(appId);
  bool isToTriggerRemovalOfPairing = false;
  if(entryOrNothing.has_value())
  {
    if(entryOrNothing.value().IsPaired())
    {
      isToTriggerRemovalOfPairing = true;
    }
  }
  if(isToTriggerRemovalOfPairing)
  {
    RemoveAppStoragePair(appId);
  }
}

void ASSP::ApplicationAdded(const unsigned int appId)
{
  auto entryOrNothing = m_sspr.GetEntry(appId);
  bool isToTriggerPairing = true;
  if(entryOrNothing.has_value())
  {
    if(entryOrNothing.value().IsPaired())
    {
      isToTriggerPairing = false;
      LogMsg(LogPrioInfo, "ASSP::ApplicationAdded AppId %u already paired",appId);
      entryOrNothing.value().Print();  
    }
  }
  
  if(isToTriggerPairing)
  {
    PairAppWithStorageIfPossible(appId);
  }
}

void ASSP::StorageRemoved(const unsigned int storageId)
{
  std::vector<unsigned int> appIds;
  m_sspr.GetAllAppIdThatUseThisStorageId(storageId, appIds);
  if(!appIds.empty())
  {
    for( auto appId : appIds)
    {
      RemoveAppStoragePair(appId);
    }
  }
}
void ASSP::StorageAdded(const unsigned int storageId)
{
  CheckForUnPairedAppAndTryToPair();
}

  
void ASSP::Print() const
{
  LogMsg(LogPrioInfo,"--- >ASSP< ---");
 
  LogMsg(LogPrioInfo,"--- <ASSP> ---");
}
