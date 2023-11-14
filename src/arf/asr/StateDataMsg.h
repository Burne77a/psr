#ifndef ARF_ASR_STATEDATAMSG_H
#define ARF_ASR_STATEDATAMSG_H

#include "ISerializable.h"
#include <string>
#include <memory>

class StateDataMsg : public ISerializable
{
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
    ~StateDataMsg();
    
    void SetPayload(ISerializable &objToSend);
    
    //ISerializable
    uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
  private:
    mutable StateData m_data;
    std::vector<uint8_t> m_dataAndPayloadSerialized;
    static const unsigned int MAX_PAYLOAD_SIZE{3000U};
};

#endif //ARF_ASR_STATEDATAMSG_H
