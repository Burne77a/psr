#include "StorageInfo.h"
#include "Logger.h"
#include <algorithm>
#include <errnoLib.h>


std::shared_ptr<StorageInfo> StorageInfo::CreateFromRawPtr(const uint8_t * pBuffer, const uint32_t size)
{
  std::shared_ptr<StorageInfo> pSi{nullptr};
  if((pBuffer != nullptr) && (size >= sizeof(StorageInfoData)))
  {
    pSi = std::make_shared<StorageInfo>();
    if(pSi)
    {
      uint32_t sizeOfBufToDeSerTo = 0U;
      const uint8_t *pBufToDeSerTo = pSi->GetSerializableDataBuffer(sizeOfBufToDeSerTo);
      if((pBufToDeSerTo != nullptr) && (size <= sizeOfBufToDeSerTo))
      {
        std::memcpy((void*)pBufToDeSerTo,(void*)pBuffer,size);
        if(!pSi->Deserialize())
        {
          LogMsg(LogPrioError, "ERROR: StorageInfo::CreateFromRawPtr failed to deserialize Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
          pSi = nullptr;
        }
      }
      else
      {
        pSi = nullptr;
        LogMsg(LogPrioError, "ERROR: StorageInfo::CreateFromRawPtr invalid buffers to deserialize to 0x% %u &u Errno: 0x%x (%s)",pBufToDeSerTo,size,sizeOfBufToDeSerTo ,errnoGet(),strerror(errnoGet()));
      }
    }
    else
    {
      LogMsg(LogPrioError, "ERROR: StorageInfo::CreateFromRawPtr failed to create pointer Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: StorageInfo::CreateFromRawPtr No valid ClientMsg data in LR 0x%x %u %u  Errno: 0x%x (%s)",pBuffer,size,sizeof(StorageInfoData),errnoGet(),strerror(errnoGet()));
  }
  return pSi; 
}


StorageInfo::StorageInfo(const unsigned int storageId, std::string_view ipAddr, const unsigned int sizeInBytes, const unsigned int bandwidth, const unsigned int nodeId)
{
  m_data.storageId = storageId;
  m_data.sizeInBytes = sizeInBytes;
  m_data.bandwidth = bandwidth;
  m_data.nodeId = nodeId;
  size_t lengthToCopy = std::min(ipAddr.size(), sizeof(m_data.ipAddr) - 1);
  std::copy_n(ipAddr.begin(), lengthToCopy, m_data.ipAddr);
  m_data.ipAddr[lengthToCopy] = '\0';
}

StorageInfo::StorageInfo(const unsigned int storageId)
{
  m_data.ipAddr[0] = '\0';
  m_data.storageId = storageId;
}

StorageInfo::StorageInfo()
{
  m_data.ipAddr[0] = '\0';
}

const uint8_t *  StorageInfo::Serialize(uint32_t &size) const 
{
  size = sizeof(m_data);
  return (uint8_t *)&m_data;
}

const uint8_t * StorageInfo::GetSerializableDataBuffer(uint32_t &size) const
{
  return Serialize(size);
}

bool StorageInfo::Deserialize()
{
  return true;
}



void StorageInfo::Print() const
{
  LogMsg(LogPrioInfo,"StorageInfo: ID %u NodeId: %u ip: %s Size: %u, Bandwidth %u",m_data.storageId,m_data.nodeId,m_data.ipAddr,m_data.sizeInBytes, m_data.bandwidth);
}


