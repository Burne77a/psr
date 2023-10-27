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

void ServiceUpcallDispatcher::Print() const
{
  LogMsg(LogPrioInfo, "--- ServiceUpcallDispatcher ---");
  
  for(auto & [key,cb] : m_upcallCbs)
  {
    LogMsg(LogPrioInfo, "Service ID %d has registered callback",key);
  }
  
  LogMsg(LogPrioInfo, "--- --- ---");
}
