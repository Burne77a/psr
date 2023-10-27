#include "ClientMessage.h"

ClientMessage::ClientMessage(const ISerializable &payload, const unsigned int serviceId, const ClientRequestId reqId) : m_serviceId{serviceId}, m_payload{payload}, m_reqId{reqId}
{
  
}

uint8_t *  ClientMessage::Serialize(uint32_t &size) const
{
  return nullptr;
}

const uint8_t * ClientMessage::GetSerializableDataBuffer(uint32_t &size) const 
{
  return nullptr;
}
   
bool ClientMessage::Deserialize()
{
  return true;
}
   

