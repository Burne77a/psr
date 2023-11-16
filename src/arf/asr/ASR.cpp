#include "ASR.h"
#include "Logger.h"
#include "NwAid.h"
#include <errnoLib.h>

static const int APP_STORE_PORT_BASE_RETRIEVER = 9200;
static const int APP_STORE_PORT_BASE_KEEPER = 9400;

std::unique_ptr<ASR> ASR::CreateASR(const unsigned int appId, const unsigned int primaryNodeId, const unsigned int storageNodeId, const unsigned int thisNodeId ,std::string_view backupIp,std::string_view storageIp)
{
  const int appStatePortKeeper = APP_STORE_PORT_BASE_KEEPER + appId;
  const int appStatePortRetriever = APP_STORE_PORT_BASE_RETRIEVER + appId;
  bool isCreationOk = true;
  
  std::unique_ptr<ASR> pAsr{nullptr};
  std::unique_ptr<StateKeeper> pStateKeeper{nullptr}; 
  std::unique_ptr<StateRetriever> pStateRetriever{nullptr};
  std::unique_ptr<StateStorer> pStateStorer{nullptr};
  
  pStateStorer = StateStorer::CreateStateStorer(appId, appStatePortKeeper, storageIp);
  if(!pStateStorer)
  {
    LogMsg(LogPrioCritical, "ERROR: ASR::CreateASR failed to create StateStorer (port %d). Errno: 0x%x (%s)",appStatePortKeeper,errnoGet(),strerror(errnoGet()));
    isCreationOk = false;
  }
  
  pStateRetriever = StateRetriever::CreateStateRetriever(appId, appStatePortRetriever,appStatePortKeeper, storageIp);
  if(!pStateRetriever)
  {
    LogMsg(LogPrioCritical, "ERROR: ASR::CreateASR failed to create StateRetriever (port %d). Errno: 0x%x (%s)",appStatePortRetriever,errnoGet(),strerror(errnoGet()));
    isCreationOk = false;
  } 
  
  
  if(thisNodeId == storageNodeId)
  {
    pStateKeeper = StateKeeper::CreateStateKeeper(appId, appStatePortRetriever,appStatePortKeeper, backupIp);
    if(!pStateKeeper)
    {
      LogMsg(LogPrioCritical, "ERROR: ASR::CreateASR failed to create StateKeeper (port %d). Errno: 0x%x (%s)",appStatePortRetriever,errnoGet(),strerror(errnoGet()));
      isCreationOk = false;
    } 
  }  
  if(isCreationOk)
  {
    pAsr = std::make_unique<ASR>(appId,pStateKeeper,pStateRetriever,pStateStorer);
    if(!pAsr)
    {
      LogMsg(LogPrioCritical, "ERROR: ASR::CreateASR failed to create ASR. appId: %u  Errno: 0x%x (%s)",appId,errnoGet(),strerror(errnoGet()));
    }
  }
  return pAsr;
}

ASR::ASR(const unsigned int appId,  std::unique_ptr<StateKeeper> & pStateKeeper,std::unique_ptr<StateRetriever> & pStateRetriever,std::unique_ptr<StateStorer> &pStateStorer) 
: m_appId{appId}, m_pStateKeeper{std::move(pStateKeeper)},m_pStateRetriever{std::move(pStateRetriever)},m_pStateStorer{std::move(pStateStorer)}
{
  
}


bool ASR::PrimaryAppSendStateToStorage(const ISerializable & objToSend)
{
  bool isSuccessfullySentToStorage = false;
  if(m_pStateStorer)
  {
    if(m_pStateStorer->SendAppDataToStorage(objToSend))
    {
      isSuccessfullySentToStorage = true;
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: ASR::CreatPrimaryAppSendStateToStorage failed to send data to storage appId: %u  Errno: 0x%x (%s)",m_appId,errnoGet(),strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: ASR::CreatPrimaryAppSendStateToStorage no StateStorer installed. Is this instance primary? appId: %u",m_appId);
  }
  return isSuccessfullySentToStorage;
}

bool ASR::BackupAppGetStateFromStorage(ISerializable & objToRcvTo)
{
  bool isSuccessfullyFetched = false;
  if(m_pStateRetriever)
  {
    if(m_pStateRetriever->GetLatestStateFromStorage(objToRcvTo))
    {
      isSuccessfullyFetched = true;
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: ASR::BackupAppGetStateFromStorage failed to get data from storage appId: %u  Errno: 0x%x (%s)",m_appId,errnoGet(),strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: ASR::BackupAppGetStateFromStorage no StateRetriever installed. Is this instance backup? appId: %u",m_appId);
  }
  return isSuccessfullyFetched;
}


OSAStatusCode ASR::Start()
{
  OSAStatusCode startSts = OSA_OK;
  if(m_pStateKeeper)
  {
    startSts = m_pStateKeeper->Start();
    if(startSts != OSA_OK)
    {
      LogMsg(LogPrioCritical, "ERROR: ASR::Start failed to start StateKeeper (appId %u). Errno: 0x%x (%s)",m_appId,errnoGet(),strerror(errnoGet()));
    }
  } 
  return startSts;
}

void ASR::Stop()
{
  if(m_pStateKeeper)
  {
    m_pStateKeeper->Stop();
  }
}



  
void ASR::Print() const
{
  LogMsg(LogPrioInfo,"--- >ASR< ---");
  LogMsg(LogPrioInfo,"AppId: %u StateKeeper: %s StateRetriver: %s StateStorer: %s",m_appId,
      m_pStateKeeper ? "YES" : "NO", m_pStateRetriever ? "YES" : "NO",m_pStateStorer ? "YES" : "NO");
  if(m_pStateKeeper)
  {
    m_pStateKeeper->Print();
  }
  if(m_pStateRetriever)
  {
    m_pStateRetriever->Print();
  }
  if(m_pStateStorer)
  {
    m_pStateStorer->Print();
  }
  LogMsg(LogPrioInfo,"--- <ASR> ---");
}



