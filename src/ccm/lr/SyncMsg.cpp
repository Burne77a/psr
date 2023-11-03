#include "SyncMsg.h"
#include "Logger.h"
#include <errnoLib.h>


std::shared_ptr<SyncMsg> SyncMsg::CreateSyncMsgFromLogEntry(const int requesterId, const unsigned int numberOfEntries, const ISerializable &entry, const unsigned int currentEntryIndex)
{
  std::shared_ptr<SyncMsg> pSyncMsg = std::make_shared<SyncMsg>(requesterId,numberOfEntries,entry,currentEntryIndex);
  if(!pSyncMsg)
  {
    LogMsg(LogPrioCritical, "ERROR SyncMsg::CreateSyncMsgFromLogEntry failed to create SyncMsg. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  
  return pSyncMsg;
}

SyncMsg::SyncMsg(const int requsterId, const unsigned int numberOfEntries, const ISerializable &entry, const unsigned int currentEntryIndex)
{
  m_data.currentEntryIndex = currentEntryIndex;
  m_data.requesterId = requsterId;
  m_data.totalNumberOfEntries = numberOfEntries;
  m_data.type = MsgType::LogEntry;
  const uint8_t *pPayloadSrcBuffer = entry.Serialize(m_data.payloadSize);
  if((pPayloadSrcBuffer != nullptr) && (m_data.payloadSize > 0))
  {
    const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
    const uint32_t sizeOfMeta = sizeof(m_data);
    m_serializedDataInclPayload.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
    m_serializedDataInclPayload.insert(m_serializedDataInclPayload.end(), pPayloadSrcBuffer, pPayloadSrcBuffer + m_data.payloadSize);
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: SyncMsg::SyncMsg invalid payload 0x%x %u",pPayloadSrcBuffer,m_data.payloadSize);
  }
  
}

SyncMsg::SyncMsg(const MsgType type, const int idOfRequester)
{
  m_data.type = type;
  m_data.requesterId = idOfRequester;
  m_data.payloadSize = 0;
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_serializedDataInclPayload.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
}
SyncMsg::SyncMsg(const SyncMsg &src) : m_data{src.m_data}, m_serializedDataInclPayload{src.m_serializedDataInclPayload}
{
  //TODO: Ugly workaround. Make better later.
  m_serializedDataInclPayload.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
  std::memcpy(m_serializedDataInclPayload.data(),src.m_serializedDataInclPayload.data(),src.m_data.payloadSize + sizeof(m_data));
}

SyncMsg::SyncMsg()
{
  
}

SyncMsg::~SyncMsg()
{
  
}


uint8_t * SyncMsg::GetDeserializedPayload(uint32_t& size) const
{
  size = m_data.payloadSize;
  return (m_serializedDataInclPayload.data() + sizeof(m_data));
}

const uint8_t *  SyncMsg::Serialize(uint32_t &size) const 
{
  //Already serialized, in constructor.
  size = m_serializedDataInclPayload.size();
  return m_serializedDataInclPayload.data();
}

const uint8_t * SyncMsg::GetSerializableDataBuffer(uint32_t &size) const 
{
  m_serializedDataInclPayload.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
  size = m_serializedDataInclPayload.capacity();
  return m_serializedDataInclPayload.data();
}

bool SyncMsg::Deserialize()
{
  bool isSuccessfullyDeserialized = false;
  SyncMsgData  * pSyncMsgData = (SyncMsgData *)m_serializedDataInclPayload.data();
  if((pSyncMsgData != nullptr) && (m_serializedDataInclPayload.capacity() >= sizeof(SyncMsgData)))
  {
    m_data = *pSyncMsgData;
    isSuccessfullyDeserialized = true;
  }
  else
  {
    LogMsg(LogPrioError,"SyncMsg::Deserialize invalid incoming data 0x%x, %u",pSyncMsgData, m_serializedDataInclPayload.size());
  }
  return isSuccessfullyDeserialized;

}



static const char * const msgTypeStr[2] =  {"RequestLog","LogEntry"};
const std::string SyncMsg::GetMsgTypeAsString(const MsgType type)
{
  return std::string(msgTypeStr[(int)type]);
}

void SyncMsg::Print() const
{
  LogMsg(LogPrioInfo,"SyncMsg: type: %d (%s) RequseterId: %d payloadSize: %u entries %u:%u",m_data.type, 
      GetMsgTypeAsString(m_data.type).c_str(),m_data.requesterId,m_data.payloadSize,
      m_data.totalNumberOfEntries, m_data.currentEntryIndex);
  
  LogMsg(LogPrioInfo,"SyncMsg (debug):  size: %u data 0x%x payloadPtr 0x%x",
      m_serializedDataInclPayload.size(),m_serializedDataInclPayload.data(),(m_serializedDataInclPayload.data() + sizeof(m_data)));
}

