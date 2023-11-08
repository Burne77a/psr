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

void SSPR::HandleActivity()
{
  ProcessIncomingParingRequestIfLeaderFlushOtherwise();
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

void SSPR::DeRegisterPairing(AppStateStoragePair &pairingToDeReg)
{
  std::shared_ptr<ISerializable> pPairInfo = std::make_shared<AppStateStoragePair>(pairingToDeReg);
  if(!pPairInfo)
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::DeRegisterPairing failed to create AppStateStoragePair. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  if(PostRequestToCCM(pPairInfo,AppStateStoragePairMsg::MsgType::RemovePairing))
  {
    LogMsg(LogPrioInfo, "SSPR::DeRegisterPairing request successfully replicated.");
    pairingToDeReg.Print();
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::RegisterPairing failed to send request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  } 
}
void SSPR::RegisterPairing(AppStateStoragePair &pairingToReg)
{
  std::shared_ptr<ISerializable> pPairInfo = std::make_shared<AppStateStoragePair>(pairingToReg);
  if(!pPairInfo)
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::RegisterPairing failed to create AppStateStoragePair. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  if(PostRequestToCCM(pPairInfo,AppStateStoragePairMsg::MsgType::AddPairing))
  {
    LogMsg(LogPrioInfo, "SSPR::RegisterPairing request successfully replicated.");
    pairingToReg.Print();
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::RegisterPairing failed to send request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
}

bool SSPR::PostRequestToCCM(const std::shared_ptr<ISerializable>& pPayload, const AppStateStoragePairMsg::MsgType type)
{
  bool isRequestSuccessfullyPosted = false;
  std::shared_ptr<ISerializable> pSerPay = std::make_shared<AppStateStoragePairMsg>(pPayload,type);
  if(!pSerPay)
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::PostRequestToCCM failed to create AppStateStoragePairMsg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return false;
  }
  ClientMessage cm{pSerPay,SERVICE_ID,m_pIccm->CreateUniqueId()};
  if(m_pIccm->ReplicateRequest(cm))
  {
    isRequestSuccessfullyPosted = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: pSerPay::PostRequestToCCM failed to register request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  return isRequestSuccessfullyPosted;
}


void SSPR::HandleIncomingPairingRequest(AppStateStoragePair &incoming)
{
  if(incoming.IsPaired())
  {
    RegisterPairing(incoming);
    LogMsg(LogPrioInfo, "SSPR::HandleIncomingPairingRequest issued registration of pairing");
    incoming.Print();
  }
  else
  {
    DeRegisterPairing(incoming);
    LogMsg(LogPrioInfo, "SSPR::HandleIncomingPairingRequest issued de-registration of pairing");
    incoming.Print();
  }
}

void SSPR::ProcessIncomingParingRequestIfLeaderFlushOtherwise()
{
  bool isMsgRcvd = false;
  do
  {
    isMsgRcvd = false;
    std::unique_ptr<AppStateStoragePair> pAppStoragePair = GetStoragePairEntryFromQueue();
    if(pAppStoragePair)
    {
      isMsgRcvd = true;
      if(m_pIccm->IsFullySyncLeader())
      {
        HandleIncomingPairingRequest(*pAppStoragePair);
        //To not overload the leader, only process a message each iteration. 
        return;
      }
    }
  }while(isMsgRcvd);
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

void SSPR::HandleAddReq(AppStateStoragePairMsg& msg)
{
  std::shared_ptr<AppStateStoragePair> pAssp = msg.GetAppStateStoragePairPayload();
  if(pAssp)
  {
    if(m_pAppStateReg->AddEntry(*pAssp))
    {
      LogMsg(LogPrioInfo,"SSPR::HandleAddReq successfully added AppStateStoragePair");
      pAssp->Print();
    }
    else
    {
      LogMsg(LogPrioError,"ERROR: SSPR::HandleAddReq failed to add AppStateStoragePair");
      pAssp->Print();
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::HandleAddReq failed to create AppStateStoragePair entry from msg Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}

void SSPR::HandleRemoveReq(AppStateStoragePairMsg& msg)
{
  std::shared_ptr<AppStateStoragePair> pAssp = msg.GetAppStateStoragePairPayload();
  if(pAssp)
  {
    m_pAppStateReg->RemoveEntry(pAssp->GetAppId()); 
    LogMsg(LogPrioInfo,"SSPR::HandleRemoveReq successfully removed AppStateStoragePair");
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::HandleRemoveReq failed to create AppStateStoragePair entry from msg Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}

void SSPR::HandleUpcallMessage(AppStateStoragePairMsg& msg)
{
  if(msg.IsAddRequest())
  {
    HandleAddReq(msg);
  }
  else if(msg.IsRemoveRequest())
  {
    HandleRemoveReq(msg);
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR::HandleUpcallMessage unknown AppStateStoragePairMsg type Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}

void SSPR::UpcallMethod(const ClientMessage& commitedMsg)
{
  std::shared_ptr<AppStateStoragePairMsg> pAsspm = ICCM::CreateInstance<AppStateStoragePairMsg>(commitedMsg);
  if(pAsspm)
  {
    HandleUpcallMessage(*pAsspm);
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SSPR:UpcallMethod failed to create AppStateStoragePairMsg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    commitedMsg.Print();
  }
}

  
void SSPR::Print() const
{
  LogMsg(LogPrioInfo,"--- >SSPR< ---");
  m_pAppStateReg->Print();
  PrintQueueWithLock();
  LogMsg(LogPrioInfo,"--- <SSPR> ---");
}

void SSPR::PrintQueueWithLock() const
{
  std::lock_guard<std::mutex> lock(m_queueMutex);
  LogMsg(LogPrioInfo,"Queue size: %u",m_appStoragePairQueue.size());
  for(auto &entry : m_appStoragePairQueue)
  {
    if(entry)
    {
      entry->Print();
    }
    else
    {
      LogMsg(LogPrioInfo,"Nullptr element in queue");
    }
  }
}
