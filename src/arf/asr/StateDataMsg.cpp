#include "StateDataMsg.h"
#include "Logger.h"

StateDataMsg::StateDataMsg()
{
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_dataAndPayloadSerialized.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
}

StateDataMsg::~StateDataMsg()
{
  
}

void StateDataMsg::SetPayload(ISerializable &objToSend)
{
  uint8_t * pPayloadData = objToSend.Serialize(m_data.payloadSize);
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_dataAndPayloadSerialized.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
  if((pStartOfMeta != nullptr) && (m_data.payloadSize > 0))
  {
    m_dataAndPayloadSerialized.insert(m_dataAndPayloadSerialized.end(), pPayloadData, pPayloadData + m_data.payloadSize);
  }
}

uint8_t *  StateDataMsg::Serialize(uint32_t &size) const
{
  //TODO: adapt to endian differences, for now we just run little endian. 
  size = m_dataAndPayloadSerialized.size();
  return m_dataAndPayloadSerialized.data();
}

const uint8_t * StateDataMsg::GetSerializableDataBuffer(uint32_t &size) const
{
  m_dataAndPayloadSerialized.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
   size = m_dataAndPayloadSerialized.capacity();
   return m_dataAndPayloadSerialized.data();
}

bool StateDataMsg::Deserialize()
{
#warning fix this
  
  if()
  m_dataAndPayloadSerialized.data()
  return true;
}


