#ifndef ARF_AFD_HEARBEAT_AFD_H
#define ARF_AFD_HEARBEAT_AFD_H

#include "ISerializable.h"
#include <string>

class HeartbeatAFD : public ISerializable
{
  private:
     
    typedef struct
    {
     uint32_t m_senderAppId;   
    }HeartbeatData;
    
  public:
    HeartbeatAFD();
    ~HeartbeatAFD();
    
    void SetSenderAppId(const unsigned int appId) {m_dataToExchange.m_senderAppId = appId;}
    int GetSenderAppId() const {return m_dataToExchange.m_senderAppId;}
    
    //ISerializable
    uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
    
    static void InitHeartbeatData(HeartbeatData &hbData);
  private:
    mutable HeartbeatData m_dataToExchange;
    
  
};

#endif //ARF_AFD_HEARBEAT_AFD_H
