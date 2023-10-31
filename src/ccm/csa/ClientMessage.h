#ifndef CCM_CLIENTMESSAGE_H
#define CCM_CLIENTMESSAGE_H
#include "ISerializable.h"
#include "ClientRequestId.h"
#include <functional>
#include <memory>
#include <vector>

class ClientMessage : public ISerializable
{
   public:
    enum class MsgType
     {
       Request = 0,
       ACK,
       NACK
     };   
  private:
    struct MsgInfo
    {
      unsigned int m_serviceId{0U};
      ClientRequestId m_reqId{};
      MsgType m_type{MsgType::NACK};
    };
  public:
    ClientMessage();
    ClientMessage(const MsgType typeOfMsg, const ClientRequestId reqId);
    ClientMessage(std::shared_ptr<ISerializable> pPayload, const unsigned int serviceId, const ClientRequestId reqId);
    ~ClientMessage() = default;
    unsigned int GetServiceId() const {return m_msgInfo.m_serviceId;}
    ClientRequestId GetReqId() const {return m_msgInfo.m_reqId;}
    MsgType GetType() const {return m_msgInfo.m_type;}
    bool IsRequest() const {return m_msgInfo.m_type == MsgType::Request;}
    bool IsAck() const {return m_msgInfo.m_type == MsgType::ACK;}
    bool IsNAck() const {return m_msgInfo.m_type == MsgType::NACK;}
    uint8_t * GetDeserializedPayload(uint32_t& size) const;
    
    
    //ISerializable
    uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
    static const std::string GetMsgTypeAsString(const MsgType type);
    void Print() const;
  private:
    MsgInfo m_msgInfo;
    std::shared_ptr<ISerializable> m_pPayload{nullptr};
    mutable std::vector<uint8_t> m_serializedDataInclMsgAndPayload;
    uint8_t * m_pPayloadBuffer{nullptr};
    uint32_t m_payloadBufferDataSize{0U};

    
};

#endif //CCM_CLIENTMESSAGE_H
