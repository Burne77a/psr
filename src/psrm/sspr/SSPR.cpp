#include "SSPR.h"
#include "Logger.h"
#include <functional>
#include <errnoLib.h>

std::unique_ptr<SSPR> SSPR::CreateSSPR(std::shared_ptr<ICCM>& pIccm)
{
  
  std::unique_ptr<AppStateStoragePairReg> pAppStateStorageReg = std::make_unique<AppStateStoragePairReg>();
  if(!pAppStateStorageReg)
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::CreateSR failed to create AppStateStoragePairReg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  std::unique_ptr<SSPR> pSspr{nullptr};
  
  pSspr = std::make_unique<SSPR>(pIccm,pAppStateStorageReg);
  if(!pSspr)
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::CreateSSPR failed to create SSPR. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
    
  return pSspr;
}

SSPR::SSPR(std::shared_ptr<ICCM>& pIccm,std::unique_ptr<AppStateStoragePairReg>& pAppStateReg) : m_pIccm{pIccm}, m_pAppStateReg{std::move(pAppStateReg)}
{
  
}


bool SSPR::RegisterWithCCM()
{
  bool isSuccessfullyRegistered = false;
  if(m_pIccm->RegisterService(SERVICE_ID, std::bind(&SSPR::UpcallMethod,this, std::placeholders::_1)))
  {
    LogMsg(LogPrioInfo,"SSPR successfully registered with CCM. ServiceId %u",SERVICE_ID);
    isSuccessfullyRegistered = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::RegisterWithCCM failed to register with CCM. ServiceID %u. Errno: 0x%x (%s)",SERVICE_ID,errnoGet(),strerror(errnoGet()));
  }
  return isSuccessfullyRegistered;
}

void SSPR::PostStoragePairForReplication(const AppStateStoragePair &pair)
{
  std::lock_guard<std::mutex> lock(m_queueMutex);
  std::unique_ptr<AppStateStoragePair> pNewEntry = std::make_unique<AppStateStoragePair>(pair);
  if(pNewEntry)
  {
    m_appStoragePairQueue.push_back(std::move(pNewEntry));
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::PostStoragePairForReplication to create entry Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    pair.Print();
  }
}

std::unique_ptr<AppStateStoragePair> SSPR::GetStoragePairEntryFromQueue()
{
  std::lock_guard<std::mutex> lock(m_queueMutex);
  std::unique_ptr<AppStateStoragePair> pEntryToReturn{nullptr};
  if(!m_appStoragePairQueue.empty())
  {
    pEntryToReturn = std::move(m_appStoragePairQueue.front());
    m_appStoragePairQueue.erase(m_appStoragePairQueue.begin());
  }
  return pEntryToReturn;
}

void SSPR::UpcallMethod(const ClientMessage& commitedMsg)
{
  /*std::shared_ptr<StorageInfoMsg> pSim = ICCM::CreateInstance<StorageInfoMsg>(commitedMsg);
  if(pSim)
  {
    HandleUpcallMessage(*pSim);
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SR:UpcallMethod failed to create StorageInfoMsg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    commitedMsg.Print();
  }*/
}

  
void SSPR::Print() const
{
  LogMsg(LogPrioInfo,"--- >SSPR< ---");
  m_pAppStateReg->Print();
  LogMsg(LogPrioInfo,"--- <SSPR> ---");
}
