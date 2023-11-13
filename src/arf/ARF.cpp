#include "ARF.h"
#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<ARF> ARF::CreateARF(PSRM &psrm)
{
    
  std::unique_ptr<ARF> pArf = std::make_unique<ARF>(psrm);
  if(!pArf)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::CreateARF failed to create ARF. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  return pArf;
}

ARF::ARF(PSRM &psrm) : m_psrm{psrm}
{
  
}

bool ARF::RegisterAppForFD(const unsigned int appId, std::string_view primaryIp, std::string_view backupIp,const unsigned int hbTmoTimeInMs,
    PrimaryHbTimeoutCb hbTmoCb)
{
  if(m_appFailuredetectors.find(appId) != m_appFailuredetectors.end())
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::RegisterApp Id %u already in use",appId);
    return false;
  }
  
  std::unique_ptr<AFD> pAfd = AFD::CreateAFD(appId, primaryIp, backupIp, hbTmoTimeInMs,hbTmoCb);
  if(!pAfd)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::RegisterApp failed to create AFD for Id %u Errno: 0x%x (%s)",appId,errnoGet(),strerror(errnoGet()));
    return false;
  }
  
  const OSAStatusCode startSts = pAfd->Start();
  if(startSts != OSA_OK)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::RegisterApp failed to create AFD for Id %u Status: 0x%x Errno: 0x%x (%s)",appId,startSts,errnoGet(),strerror(errnoGet()));
    return false;
  }
  
  m_appFailuredetectors.insert(std::make_pair(appId, std::move(pAfd)));
  return true;
}

bool ARF::RegisterAppForStateStorage(const unsigned int appId, const unsigned int primaryNodeId, const unsigned int bytes, const unsigned int periodInMs)
{
  return m_psrm.RegisterApplication(appId, primaryNodeId, bytes, periodInMs);
}

bool ARF::SetAsPrimary(const unsigned int appId)
{
  auto entry = m_appFailuredetectors.find(appId);
  if(entry == m_appFailuredetectors.end())
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::SetAsPrimary could not find AFD for ID %u",appId);
    return false;
  }
  if(!entry->second)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::SetAsPrimary no valid AFD pointer for ID %u",appId);
    return false;
  }
  entry->second->SetStatePrimary();
  return true;
}

bool ARF::SetAsBackup(const unsigned int appId)
{
  auto entry = m_appFailuredetectors.find(appId);
  if(entry == m_appFailuredetectors.end())
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::SetAsBackup could not find AFD for ID %u",appId);
    return false;
  }
  if(!entry->second)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::SetAsBackup no valid AFD pointer for ID %u",appId);
    return false;
  }
  entry->second->SetStateBackup();
  return true;
}


bool ARF::Kickwatchdog(const unsigned int appId)
{
  auto entry = m_appFailuredetectors.find(appId);
  if(entry == m_appFailuredetectors.end())
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::Kickwatchdog could not find AFD for ID %u",appId);
    return false;
  }
  if(!entry->second)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::Kickwatchdog no valid AFD pointer for ID %u",appId);
    return false;
  }
  
  entry->second->Kickwatchdog();
  return true;
}

void ARF::PrimaryFailureTimeoutCb(const unsigned int appId)
{
  
}

void ARF::Print() const
{
  LogMsg(LogPrioInfo, "--- >ARF<---");
  LogMsg(LogPrioInfo,"Number of AFDs: %d", m_appFailuredetectors.size());
  for(auto &entry : m_appFailuredetectors)
  {
    if(entry.second)
    {
      entry.second->Print();
    }
  }

  LogMsg(LogPrioInfo, "--- <ARF> ---");
}
