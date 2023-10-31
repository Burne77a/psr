#ifndef CCM_HEARBEAT_H
#define CCM_HEARBEAT_H

#include "ISerializable.h"
#include "../gmm/Member.h" //Lazy solution for now, just used for the MAX_MEMBERS declaration
#include <string>




class HeartbeatCCM : public ISerializable
{
  private:
    typedef struct
    {
      std::string m_dstIp;
      std::string m_srcIp;
      bool m_isOutbound;
    }HeartbeatCCMMeta;
    
    typedef struct
    {
     int32_t m_senderId;   
     uint64_t m_connectionPerceptionBits;
     int32_t m_leaderId;
     uint32_t m_viewNo;
    }HeartbeatData;
    
  public:
    
    HeartbeatCCM();
    ~HeartbeatCCM();
    
    void SetConnectionPerception(std::bitset<MAX_MEMBERS> & connectionPerception){m_connectionPerception = connectionPerception;}
    std::bitset<MAX_MEMBERS> GetConnectionPerception() const {return m_connectionPerception;}
    void SetSenderId(const int id) {m_dataToExchange.m_senderId = id;}
    void SetLeaderId(const int leaderId){m_dataToExchange.m_leaderId = leaderId;}
    void SetViewNumber(const unsigned int viewNumber){m_dataToExchange.m_viewNo = viewNumber;}
    int GetSenderId() const {return m_dataToExchange.m_senderId;}
    int GetLeaderId() const {return m_dataToExchange.m_leaderId;}
    unsigned int GetViewNumber() const {return m_dataToExchange.m_viewNo;}
    
    //ISerializable
    uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
    
    //Meta related
    void SetSrcIp(const std::string & srcIp) {m_meta.m_srcIp = srcIp;}
    void SetDstIp(const std::string & dstIp) {m_meta.m_dstIp = dstIp;}
    void SetOutbound(const bool isOutbound) {m_meta.m_isOutbound = isOutbound;}
    std::string GetSrcIp(const std::string & srcIp) {return m_meta.m_srcIp;}
    std::string GetDstIp(const std::string & dstIp) {return m_meta.m_dstIp;}
    bool IsOutbound(const bool isOutbound) {return m_meta.m_isOutbound;}
    static void InitHeartbeatData(HeartbeatData &hbData);
  private:
    HeartbeatCCMMeta m_meta;
    std::bitset<MAX_MEMBERS> m_connectionPerception;
    mutable HeartbeatData m_dataToExchange;
    
  
};

#endif //CCM_HEARBEAT_H
