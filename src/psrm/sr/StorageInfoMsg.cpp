#include "StorageInfoMsg.h"
#include "Logger.h"

StorageInfoMsg::StorageInfoMsg(const uint8_t * pBuffer, const uint32_t size)
{
  if(size >= sizeof(m_data) && pBuffer != nullptr)
  {
    m_data = *(StorageInfoMsgData*)pBuffer;
    m_serializedDataInclMsgAndPayload.assign(pBuffer, pBuffer + size);
  }
  else
  {
    LogMsg(LogPrioError,"ERROR: StorageInfoMsg::StorageInfoMsg invalid payload or size. 0x%x %u %u ",pBuffer,size,sizeof(m_data));
  }
}

StorageInfoMsg::StorageInfoMsg(const std::shared_ptr<ISerializable>& pPayload, MsgType type ) : m_pPayload{pPayload}
{
  m_data.type = type;
  m_data.payloadSize = 0U;
  if(m_pPayload)
  {
    (void) m_pPayload->Serialize(m_data.payloadSize);
  }
}


std::shared_ptr<StorageInfo> StorageInfoMsg::GetStorageInfoPayload() const
{
  const unsigned int payloadSize = m_data.payloadSize;
  std::shared_ptr<StorageInfo> pSi{nullptr};
  if(payloadSize > 0)
  {
    const uint8_t *pRawPayload = m_serializedDataInclMsgAndPayload.data() + sizeof(m_data);
    pSi = StorageInfo::CreateFromRawPtr(pRawPayload,payloadSize);
    if(!pSi)
    {
      LogMsg(LogPrioError, "ERROR: StorageInfoMsg::GetAppInfoPayload failed to create StorageInfo Payload ");
    }
  }
  return pSi;
}

const uint8_t *  StorageInfoMsg::Serialize(uint32_t &size) const 
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
        LogMsg(LogPrioError, "ERROR: StorageInfoMsg::Serialize top large payload size 0x%x %u",pPayloadBufferStart,payloadSize);
      }

    }
    else
    {
      LogMsg(LogPrioError, "ERROR: StorageInfo::Serialize invalid payload ptr and/or size 0x%x %u",pPayloadBufferStart,payloadSize);
    }
  }
  size = m_serializedDataInclMsgAndPayload.size();
  return m_serializedDataInclMsgAndPayload.data();
}

const uint8_t * StorageInfoMsg::GetSerializableDataBuffer(uint32_t &size) const 
{
  m_serializedDataInclMsgAndPayload.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
  size = m_serializedDataInclMsgAndPayload.capacity();
  return m_serializedDataInclMsgAndPayload.data();
}

bool StorageInfoMsg::Deserialize()  
{
  bool isSuccessfullyDeserialized = false;
  StorageInfoMsgData * pMetaData = (StorageInfoMsgData *)m_serializedDataInclMsgAndPayload.data();
  if((pMetaData != nullptr) && (m_serializedDataInclMsgAndPayload.capacity() >= sizeof(m_data)))
  {
    m_data = *pMetaData;
    
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: StorageInfoMsg::Deserialize no valid data to deserialize ");
  }
  return isSuccessfullyDeserialized;
}

static const char * const msgTypeStr[2] =  {"AddRequest","RemoveRequest"};
const std::string StorageInfoMsg::GetMsgTypeAsString(const MsgType type)
{
  return std::string(msgTypeStr[(int)type]);
}

void StorageInfoMsg::Print() const
{
  LogMsg(LogPrioInfo,"StorageInfoMsg type: %d (%s) payloadSize %u m_pPayload ptr %s",
      m_data.type,GetMsgTypeAsString(m_data.type).c_str(),m_data.payloadSize,m_pPayload?"VALID":"INVALID");
}
