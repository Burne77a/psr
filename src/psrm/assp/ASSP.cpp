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
  
  //m_ccmIf.RegisterService(TestClientServiceId, std::bind(&TestClient::UpcallMethod,this, std::placeholders::_1));
  
  
  return pAssp;
}

ASSP::ASSP(std::shared_ptr<ICCM>& pIccm,const AIR &air, const SR &sr,SSPR &sspr) : m_pIccm{pIccm},m_air{air}, m_sr{sr},m_sspr{sspr}
{
  
}

void ASSP::PairAppWithStorageIfPossible(const unsigned int appId)
{
  //Find lowest used state storage
  unsigned int useCnt = 0U;
  bool isMatchMade = false;
  auto storageIdOrNothing = m_sspr.FindLowestUsedStateStorage(useCnt);
  if(storageIdOrNothing.has_value())
  {
    unsigned int storageId = storageIdOrNothing.value();
    if(useCnt < STORAGE_USE_CNT_LIMIT)
    {
      //Do the pairing...
    }
  }
  if(isMatchMade)
  {
    LogMsg(LogPrioWarning, "WARNING: ASSP::PairAppWithStorageIfPossible Not able to find storage for app %u",appId);
  }
}

void ASSP::ApplicationRemoved(const unsigned int appId)
{
  
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

  
void ASSP::Print() const
{
  LogMsg(LogPrioInfo,"--- >ASSP< ---");
 
  LogMsg(LogPrioInfo,"--- <ASSP> ---");
}
