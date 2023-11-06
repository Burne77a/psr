#include "AppInfoMsg.h"
#include "Logger.h"

AppInfoMsg::AppInfoMsg(const uint8_t * pBuffer, const uint32_t size)
{
  if(size >= sizeof(m_data) && pBuffer != nullptr)
  {
    m_data = *(AppInfoMsgData*)pBuffer;
    m_serializedDataInclMsgAndPayload.assign(pBuffer, pBuffer + size);
  }
  else
  {
    LogMsg(LogPrioError,"AppInfoMsg::TestRequest invalid payload or size. 0x%x %u %u ",pBuffer,size,sizeof(m_data));
  }
}

AppInfoMsg::AppInfoMsg(const std::shared_ptr<ISerializable>& pPayload, MsgType type ) : m_pPayload{pPayload}
{
  m_data.type = type;
  m_data.payloadSize = 0U;
  if(m_pPayload)
  {
    (void) m_pPayload->Serialize(m_data.payloadSize);
  }
}


std::shared_ptr<AppInfo> AppInfoMsg::GetAppInfoPayload() const
{
  const unsigned int payloadSize = m_data.payloadSize;
  std::shared_ptr<AppInfo> pAi{nullptr};
  if(payloadSize > 0)
  {
    const uint8_t *pRawPayload = m_serializedDataInclMsgAndPayload.data() + sizeof(m_data);
    pAi = AppInfo::CreateFromRawPtr(pRawPayload,payloadSize);
    if(!pAi)
    {
      LogMsg(LogPrioError, "ERROR: AppInfoMsg::GetAppInfoPayload failed to create AppInfoPayload ");
    }
  }
  return pAi;
}

const uint8_t *  AppInfoMsg::Serialize(uint32_t &size) const 
{
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_serializedDataInclMsgAndPayload.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
  if(m_pPayload)
  {
    uint32_t payloadSize = 0U;
    const uint8_t *pPayloadBufferStart = m_pPayload->Serialize(payloadSize);
    if(pPayloadBufferStart != nullptr && payloadSize > 0)
    {
      if(payloadSize <= MAX_PAYLOAD_SIZE)
      {
        m_serializedDataInclMsgAndPayload.insert(m_serializedDataInclMsgAndPayload.end(), pPayloadBufferStart, pPayloadBufferStart + payloadSize);
      }
      else
      {
        LogMsg(LogPrioError, "ERROR: AppInfoMsg::Serialize top large payload size 0x%x %u",pPayloadBufferStart,payloadSize);
      }

    }
    else
    {
      LogMsg(LogPrioError, "ERROR: AppInfoMsg::Serialize invalid payload ptr and/or size 0x%x %u",pPayloadBufferStart,payloadSize);
    }
  }
  size = m_serializedDataInclMsgAndPayload.size();
  return m_serializedDataInclMsgAndPayload.data();
}

const uint8_t * AppInfoMsg::GetSerializableDataBuffer(uint32_t &size) const 
{
  m_serializedDataInclMsgAndPayload.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
  size = m_serializedDataInclMsgAndPayload.capacity();
  return m_serializedDataInclMsgAndPayload.data();
}

bool AppInfoMsg::Deserialize()  
{
  bool isSuccessfullyDeserialized = false;
  AppInfoMsgData * pMetaData = (AppInfoMsgData *)m_serializedDataInclMsgAndPayload.data();
  if((pMetaData != nullptr) && (m_serializedDataInclMsgAndPayload.capacity() >= sizeof(m_data)))
  {
    m_data = *pMetaData;
    
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: AppInfoMsg::Deserialize no valid data to deserialize ");
  }
  return isSuccessfullyDeserialized;
}

static const char * const msgTypeStr[2] =  {"AddRequest","RemoveRequest"};
const std::string AppInfoMsg::GetMsgTypeAsString(const MsgType type)
{
  return std::string(msgTypeStr[(int)type]);
}

void AppInfoMsg::Print() const
{
  LogMsg(LogPrioInfo,"AppInfoMsg type: %d (%s) payloadSize %u m_pPayload ptr %s",
      m_data.type,GetMsgTypeAsString(m_data.type).c_str(),m_data.payloadSize,m_pPayload?"VALID":"INVALID");
}
