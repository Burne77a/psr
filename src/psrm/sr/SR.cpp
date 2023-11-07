#include "SR.h"
#include "StorageInfoMsg.h"
#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<SR> SR::CreateSR(std::shared_ptr<ICCM>& pIccm)
{
  std::unique_ptr<SR> pSr{nullptr};
  std::unique_ptr<StorageReg> pStorageReg = std::make_unique<StorageReg>();
  if(!pStorageReg)
  {
    LogMsg(LogPrioCritical, "ERROR: SR::CreateSR failed to create StorageReg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  pSr = std::make_unique<SR>(pIccm,pStorageReg);
  if(!pSr)
  {
    LogMsg(LogPrioCritical, "ERROR: SR::CreateSR failed to create SR. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  return pSr;
}

SR::SR(std::shared_ptr<ICCM>& pIccm,std::unique_ptr<StorageReg>& pStorageReg) : m_pIccm{pIccm},m_pStorageReg{std::move(pStorageReg)}
{
  
}

bool SR::RegisterWithCCM()
{
  bool isSuccessfullyRegistered = false;
  if(m_pIccm->RegisterService(SERVICE_ID, std::bind(&SR::UpcallMethod,this, std::placeholders::_1)))
  {
    LogMsg(LogPrioInfo,"SR successfully registered with CCM. ServiceId %u",SERVICE_ID);
    isSuccessfullyRegistered = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SR::RegisterWithCCM failed to register with CCM. ServiceID %u. Errno: 0x%x (%s)",SERVICE_ID,errnoGet(),strerror(errnoGet()));
  }
  return isSuccessfullyRegistered;
}

bool SR::RegisterStorage(const unsigned int storageId, const std::string_view ipAddr, const unsigned int spaceInBytes, const unsigned int bandwidth)
{
  bool isRegistrationReqSuccessfullyReplicated = false;
  std::shared_ptr<ISerializable> pStorageInfo = std::make_shared<StorageInfo>(storageId,ipAddr,spaceInBytes,bandwidth);
  if(!pStorageInfo)
  {
    LogMsg(LogPrioCritical, "ERROR: SR::RegisterApplication failed to create StorageInfo. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return false;
  }
  
  if(PostRequestToCCM(pStorageInfo,StorageInfoMsg::MsgType::AddRequest))
  {
    LogMsg(LogPrioInfo, "SR::RegisterApplication request successfully replicated. %u %s %u %u ",storageId, ipAddr.data(),spaceInBytes,bandwidth);
    isRegistrationReqSuccessfullyReplicated = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SR::RegisterApplication failed to send request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  return isRegistrationReqSuccessfullyReplicated;
}

bool SR::DeRegisterStorage(const unsigned int storageId)
{
  bool isDeregistrationReqSuccessfullyReplicated = false;
  std::shared_ptr<ISerializable> pSi = std::make_shared<StorageInfo>(storageId);
  if(!pSi)
  {
   LogMsg(LogPrioCritical, "ERROR: SR::DeRegisterStorage failed to create StorageInfo. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
   return false;
  }
  
  if(PostRequestToCCM(pSi,StorageInfoMsg::MsgType::RemoveRequest))
  {
    LogMsg(LogPrioInfo, "SR::DeRegisterStorage request successfully replicated. %u  ",storageId);
    isDeregistrationReqSuccessfullyReplicated = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SR::DeRegisterStorage failed to send request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
 
  return isDeregistrationReqSuccessfullyReplicated;
}


void SR::RegisterStorageAddedCb(StorageInfoChangeStorageRemovedCallbackType callback)
{
  if(m_storageAddedCb)
  {
    LogMsg(LogPrioWarning, "WARNING: SR::RegisterStorageAddedCb overwriting existing callback");
  }
  m_storageAddedCb = callback;
}

void SR::RegisterStorageRemovedCb(StorageInfoChangeStorageRemovedCallbackType callback)
{
  if(m_storageRemovedCb)
  {
    LogMsg(LogPrioWarning, "WARNING: SR::RegisterStorageRemovedCb overwriting existing callback");
  }
  m_storageRemovedCb = callback;
}


bool SR::PostRequestToCCM(const std::shared_ptr<ISerializable>& pPayload, const StorageInfoMsg::MsgType type)
{
  bool isRequestSuccessfullyPosted = false;
  std::shared_ptr<ISerializable> pSiM = std::make_shared<StorageInfoMsg>(pPayload,type);
  if(!pSiM)
  {
    LogMsg(LogPrioCritical, "ERROR: SR::PostRequestToCCM failed to create StorageInfoMsg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return false;
  }
  ClientMessage cm{pSiM,SERVICE_ID,m_pIccm->CreateUniqueId()};
  if(m_pIccm->ReplicateRequest(cm))
  {
    isRequestSuccessfullyPosted = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SR::PostRequestToCCM failed to register request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  return isRequestSuccessfullyPosted;
}

void SR::HandleRemoveReq(StorageInfoMsg & msg)
{
  std::shared_ptr<StorageInfo> pSi = msg.GetStorageInfoPayload();
  if(pSi)
  {
    m_pStorageReg->RemoveEntry(pSi->GetId()); 
    LogMsg(LogPrioInfo,"SR::HandleRemoveReq successfully removed storage info");
    if(m_storageRemovedCb)
    {
      m_storageRemovedCb(pSi->GetId());
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::HandleRemoveReq to create StorageInfo entry from msg Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}


void SR::HandleAddReq(StorageInfoMsg & msg)
{
  std::shared_ptr<StorageInfo> pSi = msg.GetStorageInfoPayload();
  if(pSi)
  {
    if(m_pStorageReg->AddEntry(*pSi))
    {
      LogMsg(LogPrioInfo,"SR::HandleAddReq successfully added storage info");
      pSi->Print();
      if(m_storageAddedCb)
      {
        m_storageAddedCb(pSi->GetId());
      }
    }
    else
    {
      LogMsg(LogPrioError,"ERROR: SR::HandleAddReq failed to add storage info");
      pSi->Print();
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SR::HandleAddReq to create StorageInfo entry from msg Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}

void SR::HandleUpcallMessage(StorageInfoMsg & msg)
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
    LogMsg(LogPrioCritical, "ERROR: SR::HandleUpcallMessage unknown StorageInfoMsg type Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}

void SR::UpcallMethod(const ClientMessage& commitedMsg)
{
  std::shared_ptr<StorageInfoMsg> pSim = ICCM::CreateInstance<StorageInfoMsg>(commitedMsg);
  if(pSim)
  {
    HandleUpcallMessage(*pSim);
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: SR:UpcallMethod failed to create StorageInfoMsg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    commitedMsg.Print();
  }
}
  
void SR::Print() const
{
  LogMsg(LogPrioInfo,"--- >SR< ---");
  m_pStorageReg->Print();
  LogMsg(LogPrioInfo,"--- <SR> ---");
}
