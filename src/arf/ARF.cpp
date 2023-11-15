#include "ARF.h"
#include "Logger.h"
#include <errnoLib.h>

std::unique_ptr<ARF> ARF::CreateARF(PSRM &psrm,const unsigned int backupId)
{
    
  std::unique_ptr<ARF> pArf = std::make_unique<ARF>(psrm,backupId);
  if(!pArf)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::CreateARF failed to create ARF. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  return pArf;
}

ARF::ARF(PSRM &psrm,const unsigned int backupId) : m_psrm{psrm},m_backupId{backupId}
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


bool ARF::PrimaryAppSendStateToStorage(const unsigned int appId,const ISerializable & objToSend)
{
  auto entry = m_appStateReplicators.find(appId);
  if(entry == m_appStateReplicators.end())
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::PrimaryAppSendStateToStorage could not find ASR for ID %u",appId);
    return false;
  }
  if(!entry->second)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::PrimaryAppSendStateToStorage no valid ASR pointer for ID %u",appId);
    return false;
  }
  
  return entry->second->PrimaryAppSendStateToStorage(objToSend);
}

bool ARF::BackupAppGetStateFromStorage(const unsigned int appId,ISerializable & objToRcvTo)
{
  auto entry = m_appStateReplicators.find(appId);
  if(entry == m_appStateReplicators.end())
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::BackupAppGetStateFromStorage could not find ASR for ID %u",appId);
    return false;
  }
  if(!entry->second)
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::BackupAppGetStateFromStorage no valid ASR pointer for ID %u",appId);
    return false;
  }
  
  return entry->second->BackupAppGetStateFromStorage(objToRcvTo);
}


void ARF::StateStorageChangeCallback(const AppStateStoragePair &affectedPair, bool isRemoved)
{
  const unsigned int myId = m_psrm.GetMyId();
  const unsigned int appId = affectedPair.GetAppId();
  const unsigned int primaryId = affectedPair.GetPrimaryAppNodeId();
  const unsigned int storageId = affectedPair.GetStorageId();
  const unsigned int backupId = m_backupId;
 
  //Is this node affected?
  if((myId == primaryId) || (myId == storageId) || (myId == backupId))
  {
    std::string backupIp{m_psrm.GetIp(backupId)};
    CreateAndInstallASR(appId,primaryId,storageId,myId,backupIp,affectedPair.GetIpAddr());
  }
}


void ARF::CreateAndInstallASR(const unsigned int appId, const unsigned int primaryNodeId, const unsigned int storageNodeId, const unsigned int thisNodeId ,std::string_view backupIp,std::string_view storageIp)
{
  if(m_appStateReplicators.find(appId) == m_appStateReplicators.end())
  {
    std::unique_ptr<ASR> pAsr = ASR::CreateASR(appId, primaryNodeId, storageNodeId,thisNodeId, backupIp,storageIp);
    if(pAsr)
    {
      if(pAsr->Start() == OSA_OK)
      {
        m_appStateReplicators.insert(std::make_pair(appId, std::move(pAsr)));
      }
      else
      {
        LogMsg(LogPrioCritical, "ERROR: ARF::CreateAndInstallASR failed to start ASR for Id %u Errno: 0x%x (%s)",appId,errnoGet(),strerror(errnoGet()));
      }
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: ARF::CreateAndInstallASR failed to create ASR for Id %u Errno: 0x%x (%s)",appId,errnoGet(),strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: ARF::StateStorageChangeCallback Id %u already in use",appId);
  }
}

void ARF::Print() const
{
  LogMsg(LogPrioInfo, "--- >ARF< ---");
  LogMsg(LogPrioInfo,"Number of AFDs: %d Number of ASRs: %d BackupId: %u", m_appFailuredetectors.size(),m_appStateReplicators.size(),m_backupId);
  for(auto &entry : m_appFailuredetectors)
  {
    if(entry.second)
    {
      entry.second->Print();
    }
  }
  
  for(auto &entry : m_appStateReplicators)
  {
    if(entry.second)
    {
      entry.second->Print();
    }
  }

  LogMsg(LogPrioInfo, "--- <ARF> ---");
}
