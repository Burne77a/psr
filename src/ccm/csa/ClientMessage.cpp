#include "ClientMessage.h"
#include "Logger.h"
#include "../lr/LogReplicationMsg.h"
#include <errnoLib.h>


std::shared_ptr<ClientMessage> ClientMessage::CreateClientMessage(const uint8_t *pClientData, const uint32_t clientDataSize)
{
  std::shared_ptr<ClientMessage> pCm{nullptr};
  if((pClientData != nullptr) && (clientDataSize >= sizeof(MsgInfo)))
  {
    pCm = std::make_shared<ClientMessage>();
    if(pCm)
    {
      uint32_t sizeOfBufToDeSerTo = 0U;
      const uint8_t *pBufToDeSerTo = pCm->GetSerializableDataBuffer(sizeOfBufToDeSerTo);
      if((pBufToDeSerTo != nullptr) && (clientDataSize <= sizeOfBufToDeSerTo))
      {
        std::memcpy((void*)pBufToDeSerTo,(void*)pClientData,clientDataSize);
        if(!pCm->Deserialize())
        {
          LogMsg(LogPrioError, "ERROR: ClientMessage::CreateClientMessage failed to deserialize Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
          pCm = nullptr;
        }
      }
      else
      {
        pCm = nullptr;
        LogMsg(LogPrioError, "ERROR: ClientMessage::CreateClientMessage invalid buffers to deserialize to 0x% %u &u Errno: 0x%x (%s)",pBufToDeSerTo,clientDataSize,sizeOfBufToDeSerTo ,errnoGet(),strerror(errnoGet()));
      }
    }
    else
    {
      LogMsg(LogPrioError, "ERROR: ClientMessage::CreateClientMessage failed to create pointer Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: ClientMessage::CreateClientMessage No valid ClientMsg data in LR 0x%x %u %u  Errno: 0x%x (%s)",pClientData,clientDataSize,sizeof(MsgInfo),errnoGet(),strerror(errnoGet()));
  }
  return pCm; 
}

std::shared_ptr<ClientMessage> ClientMessage::CreateClientMessage(const LogReplicationMsg &msg)
{
  std::shared_ptr<ClientMessage> pCm{nullptr};
  
  uint32_t lrPayloadSizeRaw = 0U;
  uint8_t * const pLrPayloadRaw = msg.GetPayloadData(lrPayloadSizeRaw);
  pCm = CreateClientMessage(pLrPayloadRaw,lrPayloadSizeRaw);
    
  return pCm;
}


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
  (void)m_pPayload->GetSerializableDataBuffer(m_msgInfo.m_payloadSize); //To update payload size
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
      if(payloadSize <= MAX_PAYLOAD_SIZE)
      {
        m_serializedDataInclMsgAndPayload.insert(m_serializedDataInclMsgAndPayload.end(), pPayloadBufferStart, pPayloadBufferStart + payloadSize);
        m_msgInfo.m_payloadSize = payloadSize;
      }
      else
      {
        LogMsg(LogPrioError, "ERROR: ClientMessage::Serialize %u, Req: %s to large payload size 0x%x %u",m_msgInfo.m_serviceId,m_msgInfo.m_reqId.GetIdAsStr().c_str(),pPayloadBufferStart,payloadSize);
      }

    }
    else
    {
      LogMsg(LogPrioError, "ERROR: ClientMessage::Serialize %u, Req: %s invalid payload ptr and/or size 0x%x %u",m_msgInfo.m_serviceId,m_msgInfo.m_reqId.GetIdAsStr().c_str(),pPayloadBufferStart,payloadSize);
    }
  }
  else if((m_pPayloadBuffer != nullptr) && (m_payloadBufferDataSize > 0))
  {
    m_serializedDataInclMsgAndPayload.insert(m_serializedDataInclMsgAndPayload.end(), m_pPayloadBuffer, m_pPayloadBuffer + m_payloadBufferDataSize);
    m_msgInfo.m_payloadSize =  m_payloadBufferDataSize;
  }
  
  size = m_serializedDataInclMsgAndPayload.size();
  return m_serializedDataInclMsgAndPayload.data();
}

const uint8_t * ClientMessage::GetSerializableDataBuffer(uint32_t &size) const 
{
  m_serializedDataInclMsgAndPayload.reserve(MAX_PAYLOAD_SIZE + sizeof(m_msgInfo));
  size = m_serializedDataInclMsgAndPayload.capacity();
  return m_serializedDataInclMsgAndPayload.data();
}
   
bool ClientMessage::Deserialize()
{
  bool isSuccessfullyDeserialized = false;
  MsgInfo * pMetaData = (MsgInfo *)m_serializedDataInclMsgAndPayload.data();
  if((pMetaData != nullptr) && (m_serializedDataInclMsgAndPayload.capacity() >= sizeof(m_msgInfo)))
  {
    m_msgInfo = *pMetaData;
    const unsigned int payloadSize = m_msgInfo.m_payloadSize;
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

static const char * const msgTypeStr[3] =  {" Request","ACK","NACK"};
const std::string ClientMessage::GetMsgTypeAsString(const MsgType type)
{
  return std::string(msgTypeStr[(int)type]);
}
   

void ClientMessage::Print() const
{
  LogMsg(LogPrioInfo, "--- ClientMessage ---");
  LogMsg(LogPrioInfo, "ServiceId %u, Req: %s ",m_msgInfo.m_serviceId,m_msgInfo.m_reqId.GetIdAsStr().c_str());
  LogMsg(LogPrioInfo, "Type: %s Payload size: %u",GetMsgTypeAsString(m_msgInfo.m_type).c_str(),m_msgInfo.m_payloadSize);
  LogMsg(LogPrioInfo, "m_pPayload: %s m_pPayloadBuffer: 0x%x m_payloadBufferDataSize: %u vector size: %u",m_pPayload?"Valid":"NULL",m_pPayloadBuffer,m_payloadBufferDataSize,m_serializedDataInclMsgAndPayload.size());
  
  LogMsg(LogPrioInfo, "--- --- ---");
  
}
