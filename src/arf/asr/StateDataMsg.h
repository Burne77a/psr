#ifndef ARF_ASR_STATEDATAMSG_H
#define ARF_ASR_STATEDATAMSG_H

#include "ISerializable.h"
#include <string>
#include <memory>

class StateDataMsg : public ISerializable
{
  public:
    enum class MsgType
     {
       Store = 0,
       Get,
       Fetched
     };
    
  private:
    struct StateData
    {
      MsgType type{MsgType::Store};
      uint32_t appId{0U};
      uint32_t payloadSize{0U};
    };
    
  public:
    StateDataMsg();
    StateDataMsg(const unsigned int appId, const MsgType type);
    StateDataMsg(const StateDataMsg &src);
    StateDataMsg(const MsgType type, const StateDataMsg &src);
    ~StateDataMsg();
    StateDataMsg& operator=(const StateDataMsg& other);
    
    bool IsStoreMsg() const {return m_data.type == MsgType::Store;}
    bool IsGetMsg() const {return m_data.type == MsgType::Get;}
    bool IsFetchedMsg() const {return m_data.type == MsgType::Fetched;}
    unsigned int GetAppId() const{return m_data.appId;}
    uint8_t *  GetPayloadBuffer(uint32_t &size) const;
    
    
    void SetPayload(const ISerializable &objToSend);
    
    //ISerializable
    uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
    void Print() const;
    
  private:
    mutable StateData m_data;
    mutable std::vector<uint8_t> m_dataAndPayloadSerialized;
    static const unsigned int MAX_PAYLOAD_SIZE{3000U};
};

#endif //ARF_ASR_STATEDATAMSG_H
