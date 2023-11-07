#include "AIR.h"
#include "AppInfoMsg.h"
#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<AIR> AIR::CreateAIR(std::shared_ptr<ICCM>& pIccm)
{
  std::unique_ptr<AIR> pAir{nullptr};
  std::unique_ptr<AppReg> pAppReg = std::make_unique<AppReg>();
  if(!pAppReg)
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::CreateAIR failed to create AppReg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
  
  pAir = std::make_unique<AIR>(pIccm,pAppReg);
  if(!pAir)
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::CreateAIR failed to create AIR. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  return pAir;
}

AIR::AIR(std::shared_ptr<ICCM>& pIccm,std::unique_ptr<AppReg>& pAppReg) : m_pIccm{pIccm},m_pAppReg{std::move(pAppReg)}
{
  
}

bool AIR::RegisterWithCCM()
{
  bool isSuccessfullyRegistered = false;
  if(m_pIccm->RegisterService(SERVICE_ID, std::bind(&AIR::UpcallMethod,this, std::placeholders::_1)))
  {
    LogMsg(LogPrioInfo,"AIR successfully registered with CCM. ServiceId %u",SERVICE_ID);
    isSuccessfullyRegistered = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::RegisterWithCCM failed to register with CCM. ServiceID %u. Errno: 0x%x (%s)",SERVICE_ID,errnoGet(),strerror(errnoGet()));
  }
  return isSuccessfullyRegistered;
}

bool AIR::RegisterApplication(const unsigned int appId, const unsigned int primaryInfo,const unsigned int bytes,const unsigned int periodInMs)
{
  bool isRegistrationReqSuccessfullyReplicated = false;
  std::shared_ptr<ISerializable> pAi = std::make_shared<AppInfo>(appId,primaryInfo,periodInMs,bytes);
  if(!pAi)
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::RegisterApplication failed to create AppInfo. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return false;
  }
  
  if(PostRequestToCCM(pAi,AppInfoMsg::MsgType::AddRequest))
  {
    LogMsg(LogPrioInfo, "AIR::PostRequestToCCM request successfully replicated. %u %u %u %u ",appId, primaryInfo,bytes,periodInMs);
    isRegistrationReqSuccessfullyReplicated = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::RegisterApplication failed to send request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  return isRegistrationReqSuccessfullyReplicated;
}

bool AIR::DeRegisterApplication(const unsigned int appId)
{
  bool isDeregistrationReqSuccessfullyReplicated = false;
  std::shared_ptr<ISerializable> pAi = std::make_shared<AppInfo>(appId);
  if(!pAi)
  {
   LogMsg(LogPrioCritical, "ERROR: AIR::RegisterApplication failed to create AppInfo. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
   return false;
  }
  
  if(PostRequestToCCM(pAi,AppInfoMsg::MsgType::RemoveRequest))
  {
    LogMsg(LogPrioInfo, "AIR::DeRegisterApplication request successfully replicated. %u  ",appId);
    isDeregistrationReqSuccessfullyReplicated = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::DeRegisterApplication failed to send request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
 
  return isDeregistrationReqSuccessfullyReplicated;
}

bool AIR::PostRequestToCCM(const std::shared_ptr<ISerializable>& pPayload, const AppInfoMsg::MsgType type)
{
  bool isRequestSuccessfullyPosted = false;
  std::shared_ptr<ISerializable> pAiM = std::make_shared<AppInfoMsg>(pPayload,type);
  if(!pAiM)
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::PostRequestToCCM failed to create AppInfoMsg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    return false;
  }
  ClientMessage cm{pAiM,SERVICE_ID,m_pIccm->CreateUniqueId()};
  if(m_pIccm->ReplicateRequest(cm))
  {
    isRequestSuccessfullyPosted = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::PostRequestToCCM failed to register request. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  return isRequestSuccessfullyPosted;
}

void AIR::HandleRemoveReq(AppInfoMsg & msg)
{
  std::shared_ptr<AppInfo> pAi = msg.GetAppInfoPayload();
  if(pAi)
  {
    m_pAppReg->RemoveEntry(pAi->GetId()); 
    LogMsg(LogPrioInfo,"AIR::HandleRemoveReq successfully removed application info");
    if(m_appRemovedCb)
    {
      m_appRemovedCb(pAi->GetId());
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::HandleRemoveReq to create AppInfo entry from msg Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}


void AIR::HandleAddReq(AppInfoMsg & msg)
{
  std::shared_ptr<AppInfo> pAi = msg.GetAppInfoPayload();
  if(pAi)
  {
    if(m_pAppReg->AddEntry(*pAi))
    {
      LogMsg(LogPrioInfo,"AIR::HandleAddReq successfully added application info");
      pAi->Print();
      if(m_appAddedCb)
      {
        m_appAddedCb(pAi->GetId());
      }
    }
    else
    {
      LogMsg(LogPrioError,"ERROR: AIR::HandleAddReq failed to add application info");
      pAi->Print();
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::HandleAddReq to create AppInfo entry from msg Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}

void AIR::HandleUpcallMessage(AppInfoMsg & msg)
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
    LogMsg(LogPrioCritical, "ERROR: AIR::HandleUpcallMessage unknown AppInfoMsg type Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
}

void AIR::UpcallMethod(const ClientMessage& commitedMsg)
{
  std::shared_ptr<AppInfoMsg> pAim = ICCM::CreateInstance<AppInfoMsg>(commitedMsg);
  if(pAim)
  {
    HandleUpcallMessage(*pAim);
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: AIR::UpcallMethod failed to create AppInfoMsg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    commitedMsg.Print();
  }
}

void AIR::RegisterAppAddedCb(AppInfoChangeAppAddedCallbackType callback)
{
  if(m_appAddedCb)
  {
    LogMsg(LogPrioWarning, "WARNING: AIR::RegisterAppAddedCb overwriting existing callback");
  }
  m_appAddedCb = callback;
  
}
void AIR::RegisterAppRemovedCb(AppInfoChangeAppRemovedCallbackType callback)
{
  if(m_appRemovedCb)
  {
    LogMsg(LogPrioWarning, "WARNING: AIR::RegisterAppRemovedCb overwriting existing callback");
  }
  m_appRemovedCb = callback;
  
}
  
void AIR::Print() const
{
  LogMsg(LogPrioInfo,"--- >AIR< ---");
  m_pAppReg->Print();
  LogMsg(LogPrioInfo,"--- <AIR> ---");
}
