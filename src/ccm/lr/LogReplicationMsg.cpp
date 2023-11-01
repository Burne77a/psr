#include "LogReplicationMsg.h"
#include "Logger.h"
#include <errnoLib.h>



std::unique_ptr<LogReplicationMsg> LogReplicationMsg::CreateLogReplicationPrepareMessageFromClientMessage(const ClientMessage &msg,const GMM &gmm)
{
  std::unique_ptr<LogReplicationMsg> pLrMsg = std::make_unique<LogReplicationMsg>(MsgType::Prepare,gmm.GetMyOpNumber(),gmm.GetMyViewNumber(),gmm.GetMyId(),0);
  if(pLrMsg != nullptr)
  {
    pLrMsg->SetPayload(msg);
  }
  else
  {
    LogMsg(LogPrioError,"LogReplicationMsg::CreateLogReplicationPrepareMessageFromClientMessage failed to create message Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
  }
  return pLrMsg;
}

LogReplicationMsg::LogReplicationMsg(const MsgType typeOfMsg)
{
  m_theMsgData.type = typeOfMsg;
}

LogReplicationMsg::LogReplicationMsg(const MsgType typeOfMsg, const unsigned int opNumber, const unsigned int viewNumber, const unsigned int srcId, const unsigned int dstId)
{
  m_theMsgData.type = typeOfMsg;
  m_theMsgData.operationNumber = opNumber;
  m_theMsgData.viewNumber = viewNumber;
  m_theMsgData.srcId = srcId;
  m_theMsgData.dstId = dstId;
}

void LogReplicationMsg::SetPayload(const ISerializable &payload)
{
  uint32_t size = 0U;
  const uint8_t * pData = payload.Serialize(size);
  if(pData != nullptr && size > 0)
  {
    m_payload.assign(pData, pData + size);
    m_theMsgData.payloadSize = size; 
  }
}

std::shared_ptr<ClientMessage> LogReplicationMsg::GetClientMessagePayload() const
{
  return ClientMessage::CreateClientMessage(*this);
}

uint8_t * LogReplicationMsg::GetPayloadData(uint32_t& size) const
{
  size = m_payload.size();
  return m_payload.data();
}


const uint8_t *  LogReplicationMsg::Serialize(uint32_t &size) const
{
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_theMsgData);
  const uint32_t sizeOfMeta = sizeof(m_theMsgData);
  m_serializedDataInclMsgAndPayload.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
  
  m_serializedDataInclMsgAndPayload.insert(m_serializedDataInclMsgAndPayload.end(), m_payload.begin(), m_payload.end());
  size = m_serializedDataInclMsgAndPayload.size();
  return m_serializedDataInclMsgAndPayload.data();
}

const uint8_t * LogReplicationMsg::GetSerializableDataBuffer(uint32_t &size) const
{
  m_serializedDataInclMsgAndPayload.reserve(MAX_PAYLOAD_SIZE + sizeof(m_theMsgData));
  size = m_serializedDataInclMsgAndPayload.capacity();
  return m_serializedDataInclMsgAndPayload.data();
}

bool LogReplicationMsg::Deserialize()
{
  LogReplicationMsgData * pMetaData = (LogReplicationMsgData *)m_serializedDataInclMsgAndPayload.data();
  if(pMetaData != nullptr && m_serializedDataInclMsgAndPayload.capacity() >= sizeof(m_theMsgData))
  {
  
    m_theMsgData = *pMetaData;
    const uint32_t payloadSize = m_theMsgData.payloadSize; 
    if(payloadSize > 0)
    {
      const uint8_t *pPayloadStart = m_serializedDataInclMsgAndPayload.data() + sizeof(m_theMsgData);
      m_payload.assign(pPayloadStart, pPayloadStart + payloadSize);
    }
    return true;
  }
  else
  {
    LogMsg(LogPrioError,"LogReplicationMsg::Deserialize invalid incoming data 0x%x, %u",pMetaData, m_serializedDataInclMsgAndPayload.size());
    return false;
  }
}



static const char * const msgTypeStr[3] =  {"Prepare","PrepareOK","Commit"};
const std::string LogReplicationMsg::GetMsgTypeAsString(const MsgType type)
{
  return std::string(msgTypeStr[(int)type]);
}

void LogReplicationMsg::Print() const
{
  LogMsg(LogPrioInfo,"LogReplicationMsg type: %d (%s) ViewNo: %u OpNumber: %u Src: %u Dst: %u " ,m_theMsgData.type,GetMsgTypeAsString(m_theMsgData.type).c_str(),
      m_theMsgData.viewNumber,m_theMsgData.operationNumber, m_theMsgData.srcId ,m_theMsgData.dstId);
}
