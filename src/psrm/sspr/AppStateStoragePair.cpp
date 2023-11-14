#include "AppStateStoragePair.h"
#include "Logger.h"
#include <algorithm>
#include <errnoLib.h>


std::shared_ptr<AppStateStoragePair> AppStateStoragePair::CreateFromRawPtr(const uint8_t * pBuffer, const uint32_t size)
{
  std::shared_ptr<AppStateStoragePair> pAssp{nullptr};
  if((pBuffer != nullptr) && (size >= sizeof(AppStateStorageInfoData)))
  {
    pAssp = std::make_shared<AppStateStoragePair>();
    if(pAssp)
    {
      uint32_t sizeOfBufToDeSerTo = 0U;
      const uint8_t *pBufToDeSerTo = pAssp->GetSerializableDataBuffer(sizeOfBufToDeSerTo);
      if((pBufToDeSerTo != nullptr) && (size <= sizeOfBufToDeSerTo))
      {
        std::memcpy((void*)pBufToDeSerTo,(void*)pBuffer,size);
        if(!pAssp->Deserialize())
        {
          LogMsg(LogPrioError, "ERROR: AppStateStoragePair::CreateFromRawPtr failed to deserialize Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
          pAssp = nullptr;
        }
      }
      else
      {
        pAssp = nullptr;
        LogMsg(LogPrioError, "ERROR: AppStateStoragePair::CreateFromRawPtr invalid buffers to deserialize to 0x% %u &u Errno: 0x%x (%s)",pBufToDeSerTo,size,sizeOfBufToDeSerTo ,errnoGet(),strerror(errnoGet()));
      }
    }
    else
    {
      LogMsg(LogPrioError, "ERROR: AppStateStoragePair::CreateFromRawPtr failed to create pointer Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: AppStateStoragePair::CreateFromRawPtr No valid data in buffer 0x%x %u %u  Errno: 0x%x (%s)",pBuffer,size,sizeof(AppStateStorageInfoData),errnoGet(),strerror(errnoGet()));
  }
  return pAssp; 
}


AppStateStoragePair::AppStateStoragePair(const unsigned int appId, const unsigned int storageId, std::string_view stateStorageIpAddr,const unsigned int storageNodeId)
{
  m_data.appId = appId;
  SetStorage(storageId,stateStorageIpAddr,storageNodeId);
}

AppStateStoragePair::AppStateStoragePair(const unsigned int appId)
{
  m_data.storageIpAddr[0] = '\0';
  m_data.appId = appId;
}

AppStateStoragePair::AppStateStoragePair()
{
  m_data.storageIpAddr[0] = '\0';
}

void AppStateStoragePair::SetStorage(const unsigned int storageId, std::string_view storageIpAddr,const unsigned int storageNodeId)
{
  m_data.storageId = storageId;
  m_data.storageNodeId = storageNodeId;
  size_t lengthToCopy = std::min(storageIpAddr.size(), sizeof(m_data.storageIpAddr) - 1);
  std::copy_n(storageIpAddr.begin(), lengthToCopy, m_data.storageIpAddr);
  m_data.storageIpAddr[lengthToCopy] = '\0';
}

const uint8_t *  AppStateStoragePair::Serialize(uint32_t &size) const 
{
  size = sizeof(m_data);
  return (uint8_t *)&m_data;
}

const uint8_t * AppStateStoragePair::GetSerializableDataBuffer(uint32_t &size) const
{
  return Serialize(size);
}

bool AppStateStoragePair::Deserialize()
{
  return true;
}



void AppStateStoragePair::Print() const
{
  LogMsg(LogPrioInfo,"AppStateStoragePair: AppId %u StorageId: %u NodeId: %u ip: %s",m_data.appId,m_data.storageId,m_data.storageNodeId,m_data.storageIpAddr);
}
