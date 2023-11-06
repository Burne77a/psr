#include "ServiceUpcallDispatcher.h"
#include "Logger.h"

ServiceUpcallDispatcher::ServiceUpcallDispatcher()
{
  
}


bool ServiceUpcallDispatcher::RegisterService(const unsigned int serviceId,UpcallCallbackType upcallCb)
{
  m_upcallCbs[serviceId] = upcallCb;
  LogMsg(LogPrioInfo, "ServiceUpcallDispatcher::RegisterService - registered upcall callback for service %u", serviceId);
  return true;
}

void ServiceUpcallDispatcher::MakeUpcall(std::shared_ptr<ClientMessage> pCmsg)
{
  if(pCmsg)
  {
    const unsigned int serviceId = pCmsg->GetServiceId();
    UpcallCallbackType upcallCb = m_upcallCbs[serviceId];
    if(upcallCb)
    {
      upcallCb(*pCmsg);
    }
    else
    {
      LogMsg(LogPrioError, "ServiceUpcallDispatcher::MakeUpcall - invalid UpcallCallbackType pointer, cannot make upcall %u is service registred?", serviceId);
    }
    
  }
  else
  {
    LogMsg(LogPrioError, "ServiceUpcallDispatcher::MakeUpcall - invalid ClientMessage pointer, cannot make upcall");
  }
}

void ServiceUpcallDispatcher::Print() const
{
  LogMsg(LogPrioInfo, "--- >ServiceUpcallDispatcher< ---");
  
  for(auto & [key,cb] : m_upcallCbs)
  {
    LogMsg(LogPrioInfo, "Service ID %d has registered callback",key);
  }
  
  LogMsg(LogPrioInfo, "--- <ServiceUpcallDispatcher> ---");
}
