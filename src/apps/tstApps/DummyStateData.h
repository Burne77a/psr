#ifndef APPS_DUMMY_STATE_DATA_H
#define APPS_DUMMY_STATE_DATA_H

#include "ISerializable.h"
#include <string>

class DummyStateData : public ISerializable
{
  private:
     
    struct TheData
    {
      uint8_t prePattern[10]{1,2,3,4,5,6,7,8,9,10};
      uint32_t seqNr{0U};
      char postPattern[12]{"ABCDEFGHIJ"};
      uint32_t payloadSize{0};
    };
    
  public:
    DummyStateData(const unsigned int payloadSize);
    DummyStateData();
    ~DummyStateData();
    
    void SetSeqNr(const unsigned int seqNr) {m_data.seqNr = seqNr;}
    unsigned int GetSeqNr()const {return m_data.seqNr;}
    
    //ISerializable
    uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
    void Print() const;
    

  private:
    mutable TheData m_data;
    mutable std::vector<uint8_t> m_serializedData;
    mutable std::vector<uint8_t> m_payload;
    static const unsigned int MAX_PAYLOAD_SIZE{9000};
    
  
};

#endif //APPS_DUMMY_STATE_DATA_H
