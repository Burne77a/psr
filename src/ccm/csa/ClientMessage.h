#ifndef CCM_CLIENTMESSAGE_H
#define CCM_CLIENTMESSAGE_H
#include "ISerializable.h"
#include "ClientRequestId.h"
#include <functional>
#include <memory>

class ClientMessage : public ISerializable
{
  public:
    enum class MsgType
     {
       Request = 0,
       ACK,
       NACK
     };   
    ClientMessage();
    ClientMessage(std::shared_ptr<ISerializable> pPayload, const unsigned int serviceId, const ClientRequestId reqId);
    ~ClientMessage() = default;
    unsigned int GetServiceId() const {return m_serviceId;}
    ClientRequestId GetReqId() const {return m_reqId;}
    MsgType GetType() const {return m_type;}
    bool IsRequest() const {return m_type == MsgType::Request;}
    bool IsAck() const {return m_type == MsgType::ACK;}
    bool IsNAck() const {return m_type == MsgType::NACK;}
    
    //ISerializable
    uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
    
    void Print() const;
  private:
    unsigned int m_serviceId{0U};
    std::shared_ptr<ISerializable> m_pPayload{nullptr};
    ClientRequestId m_reqId{};
    MsgType m_type{MsgType::NACK};
    
};

#endif //CCM_CLIENTMESSAGE_H
