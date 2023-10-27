#ifndef CCM_CLIENTMESSAGE_H
#define CCM_CLIENTMESSAGE_H
#include "ISerializable.h"
#include "ClientRequestId.h"
#include <functional>

class ClientMessage : public ISerializable
{
  public:
    ClientMessage(const ISerializable &payload, const unsigned int serviceId, const ClientRequestId reqId);
    ~ClientMessage() = default;
    unsigned int GetServiceId() const {return m_serviceId;}
    ClientRequestId GetReqId(){return m_reqId;}
    
    //ISerializable
    uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
  private:
    const unsigned int m_serviceId;
    const ISerializable &m_payload;
    const ClientRequestId m_reqId;
    
};

#endif //CCM_CLIENTMESSAGE_H
