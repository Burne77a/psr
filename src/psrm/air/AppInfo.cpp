#include "AppInfo.h"
#include "Logger.h"
#include <errnoLib.h>


std::shared_ptr<AppInfo> AppInfo::CreateFromRawPtr(const uint8_t * pBuffer, const uint32_t size)
{
  std::shared_ptr<AppInfo> pAi{nullptr};
  if((pBuffer != nullptr) && (size >= sizeof(AppInfoData)))
  {
    pAi = std::make_shared<AppInfo>();
    if(pAi)
    {
      uint32_t sizeOfBufToDeSerTo = 0U;
      const uint8_t *pBufToDeSerTo = pAi->GetSerializableDataBuffer(sizeOfBufToDeSerTo);
      if((pBufToDeSerTo != nullptr) && (size <= sizeOfBufToDeSerTo))
      {
        std::memcpy((void*)pBufToDeSerTo,(void*)pBuffer,size);
        if(!pAi->Deserialize())
        {
          LogMsg(LogPrioError, "ERROR: AppInfo::CreateFromRawPtr failed to deserialize Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
          pAi = nullptr;
        }
      }
      else
      {
        pAi = nullptr;
        LogMsg(LogPrioError, "ERROR: AppInfo::CreateFromRawPtr invalid buffers to deserialize to 0x% %u &u Errno: 0x%x (%s)",pBufToDeSerTo,size,sizeOfBufToDeSerTo ,errnoGet(),strerror(errnoGet()));
      }
    }
    else
    {
      LogMsg(LogPrioError, "ERROR: AppInfo::CreateFromRawPtr failed to create pointer Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: AppInfo::CreateFromRawPtr No valid ClientMsg data in LR 0x%x %u %u  Errno: 0x%x (%s)",pBuffer,size,sizeof(AppInfoData),errnoGet(),strerror(errnoGet()));
  }
  return pAi; 
}


AppInfo::AppInfo(const unsigned int appId, const unsigned int primaryNodeId, const unsigned int periodInMs, const unsigned int bytesToSyncEachPeriod)
{
  m_data.appId = appId;
  m_data.primaryNodeId = primaryNodeId;
  m_data.periodInMs = periodInMs;
  m_data.bytesToSyncEachPeriod = bytesToSyncEachPeriod;
  
}

AppInfo::AppInfo(const unsigned int appId)
{
  m_data.appId = appId;
}

AppInfo::AppInfo()
{
  
}

const uint8_t *  AppInfo::Serialize(uint32_t &size) const 
{
  size = sizeof(m_data);
  return (uint8_t *)&m_data;
}

const uint8_t * AppInfo::GetSerializableDataBuffer(uint32_t &size) const
{
  return Serialize(size);
}

bool AppInfo::Deserialize()
{
  return true;
}



void AppInfo::Print() const
{
  LogMsg(LogPrioInfo,"AppInfo: ID %u Node: %u Period: %u, size: %u",m_data.appId,m_data.primaryNodeId,m_data.periodInMs, m_data.bytesToSyncEachPeriod);
}


