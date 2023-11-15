#include "StateStorer.h"
#include "Logger.h"
#include "NwAid.h"
#include <errnoLib.h>



std::unique_ptr<StateStorer> StateStorer::CreateStateStorer(const unsigned int appId, const unsigned int port, std::string_view storageIp)
{
  std::unique_ptr<StateStorer> pSs{nullptr};
  
    
  std::unique_ptr<ISender> pSnd = NwAid::CreateUniCastSender(storageIp,port);
  if(!pSnd)
  {
    LogMsg(LogPrioCritical, "ERROR: StateStorer::CreateStateStorer failed to create ISender (port %d). Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
    return nullptr;
  }
   
  
  pSs = std::make_unique<StateStorer>(appId,pSnd);
  if(!pSs)
  {
    LogMsg(LogPrioCritical, "ERROR: StateStorer::CreateStateStorer failed to create StateStorer. appId: %u  Errno: 0x%x (%s)",port,errnoGet(),strerror(errnoGet()));
  }
  
  return pSs;
}

StateStorer::StateStorer(const unsigned int appId,std::unique_ptr<ISender> &pSender) 
: m_appId{appId},m_pSender{std::move(pSender)}
{
  
}

bool StateStorer::SendAppDataToStorage(const ISerializable & appStateData)
{
  bool isSuccessfullySent = false;
  StateDataMsg outboundMsg(m_appId,StateDataMsg::MsgType::Store);
  outboundMsg.SetPayload(appStateData);
  if(m_pSender->Send(outboundMsg))
  {
    isSuccessfullySent = true;
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: StateStorer::SendAppDataToStorage failed to send state data to storage. appId: %u  Errno: 0x%x (%s)",m_appId,errnoGet(),strerror(errnoGet()));
  }
  return isSuccessfullySent;
}

  
void StateStorer::Print() const
{
  LogMsg(LogPrioInfo,"--- >StateStorer< ---");
  LogMsg(LogPrioInfo,"AppId: %u ",m_appId);
  LogMsg(LogPrioInfo,"--- <StateStorer> ---");
}



