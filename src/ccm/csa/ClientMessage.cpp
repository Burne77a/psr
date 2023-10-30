#include "ClientMessage.h"
#include "Logger.h"

ClientMessage::ClientMessage() 
{
 
}
ClientMessage::ClientMessage(std::shared_ptr<ISerializable> pPayload, const unsigned int serviceId, const ClientRequestId reqId) : m_serviceId{serviceId}, m_pPayload{pPayload}, m_reqId{reqId}
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
   

void ClientMessage::Print() const
{
  LogMsg(LogPrioInfo, "ClientMessage: ServiceId %u, Req: %s ",m_serviceId,m_reqId.GetIdAsStr().c_str());
}
