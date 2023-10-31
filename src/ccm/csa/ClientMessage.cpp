#include "ClientMessage.h"
#include "Logger.h"

ClientMessage::ClientMessage() 
{
 
}

ClientMessage::ClientMessage(const MsgType typeOfMsg, const ClientRequestId reqId)
{
  m_msgInfo.m_type = typeOfMsg;
  m_msgInfo.m_reqId = reqId;
}

ClientMessage::ClientMessage(std::shared_ptr<ISerializable> pPayload, const unsigned int serviceId, const ClientRequestId reqId) : m_pPayload{pPayload}
{
  m_msgInfo.m_type = MsgType::Request;
  m_msgInfo.m_reqId = reqId;
  m_msgInfo.m_serviceId = serviceId;
}

uint8_t * ClientMessage::GetDeserializedPayload(uint32_t& size) const 
{
  size = m_payloadBufferDataSize;
  return m_pPayloadBuffer;
}

uint8_t *  ClientMessage::Serialize(uint32_t &size) const
{
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_msgInfo);
  const uint32_t sizeOfMeta = sizeof(m_msgInfo);
  m_serializedDataInclMsgAndPayload.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
  
  //TODO: This is very, very bad but will do for now..
  if(m_pPayload && m_pPayloadBuffer != nullptr)
  {
    LogMsg(LogPrioError, "ERROR: ClientMessage::Serialize two valid payload pointers is not handled");
  }
  
  if(m_pPayload)
  {
    uint32_t payloadSize = 0U;
    const uint8_t *pPayloadBufferStart = m_pPayload->Serialize(payloadSize);
    if(pPayloadBufferStart != nullptr && payloadSize > 0)
    {
      m_serializedDataInclMsgAndPayload.insert(m_serializedDataInclMsgAndPayload.end(), pPayloadBufferStart, pPayloadBufferStart + payloadSize);
    }
    else
    {
      LogMsg(LogPrioError, "ERROR: ClientMessage::Serialize %u, Req: %s invalid payload ptr and/or size 0x%x %u",m_msgInfo.m_serviceId,m_msgInfo.m_reqId.GetIdAsStr().c_str(),pPayloadBufferStart,payloadSize);
    }
  }
  else if((m_pPayloadBuffer != nullptr) && (m_payloadBufferDataSize > 0))
  {
    m_serializedDataInclMsgAndPayload.insert(m_serializedDataInclMsgAndPayload.end(), m_pPayloadBuffer, m_pPayloadBuffer + m_payloadBufferDataSize);
  }
  
  size = m_serializedDataInclMsgAndPayload.size();
  return m_serializedDataInclMsgAndPayload.data();
}

const uint8_t * ClientMessage::GetSerializableDataBuffer(uint32_t &size) const 
{
  return Serialize(size);
}
   
bool ClientMessage::Deserialize()
{
  bool isSuccessfullyDeserialized = false;
  MsgInfo * pMetaData = (MsgInfo *)m_serializedDataInclMsgAndPayload.data();
  if((pMetaData != nullptr) && (m_serializedDataInclMsgAndPayload.size() >= sizeof(m_msgInfo)))
  {
    const unsigned int payloadSize = m_serializedDataInclMsgAndPayload.size()- sizeof(m_msgInfo);
    
    m_msgInfo = *pMetaData;
    
    if(payloadSize > 0)
    {
      m_pPayloadBuffer = m_serializedDataInclMsgAndPayload.data() + sizeof(m_msgInfo);
      m_payloadBufferDataSize = payloadSize;
    }
    isSuccessfullyDeserialized = true;
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: ClientMessage::Deserialize no valid data to deserialize ");
  }
  return isSuccessfullyDeserialized;
}
   

void ClientMessage::Print() const
{
  LogMsg(LogPrioInfo, "--- ClientMessage ---");
  LogMsg(LogPrioInfo, "ServiceId %u, Req: %s ",m_msgInfo.m_serviceId,m_msgInfo.m_reqId.GetIdAsStr().c_str());
  LogMsg(LogPrioInfo, "Type: %s, Req: %s ",m_msgInfo.m_serviceId,m_msgInfo.m_reqId.GetIdAsStr().c_str());
  
  LogMsg(LogPrioInfo, "--- --- ---");
  
}
