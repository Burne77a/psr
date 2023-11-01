#ifndef CCM_LR_MESSAGE_H
#define CCM_LR_MESSAGE_H
#include "ISerializable.h"
#include "../csa/ClientMessage.h"
#include "../gmm/GMM.h"
#include <string>
#include <vector>
#include <memory>

class LogReplicationMsg : public ISerializable
{
  public:
  enum class MsgType
  {
    Prepare = 0,
    PrepareOK,
    Commit
  };   
  private:
    struct LogReplicationMsgData
    {
      MsgType type{MsgType::Prepare};
      unsigned int viewNumber{0U};
      unsigned int operationNumber{0U};
      unsigned int requestId{0U};
      
      unsigned int srcId{0U};
      unsigned int dstId{0U};
    };
  public:
    static std::unique_ptr<LogReplicationMsg> CreateLogReplicationPrepareMessageFromClientMessage(const ClientMessage &msg,const GMM &gmm);
    LogReplicationMsg(const MsgType typeOfMsg);
    LogReplicationMsg(const MsgType typeOfMsg, const unsigned int opNumber, const unsigned int viewNumber, const unsigned int srcId, const unsigned int dstId);
    ~LogReplicationMsg() = default;
    MsgType GetType() const {return m_theMsgData.type;} 
    static const std::string GetMsgTypeAsString(const MsgType type);
    
    unsigned int GetViewNumber() const {return m_theMsgData.viewNumber;} 
    unsigned int GetSrcId() const {return m_theMsgData.srcId;}
    unsigned int GetDstId() const {return m_theMsgData.dstId;}
    unsigned int GetOpNumber() const {return m_theMsgData.operationNumber;}
    unsigned int GetReqId() const {return m_theMsgData.requestId;}
    
    void SetViewNumber(const unsigned int viewNumber){m_theMsgData.viewNumber = viewNumber;}
    void SetSrcId(const unsigned int srcId){m_theMsgData.srcId = srcId;}
    void SetDstId(const unsigned int dstId){m_theMsgData.dstId = dstId;}
    void SetOpNumber(const unsigned int opNumber){m_theMsgData.operationNumber = opNumber;}
    
    bool IsPrepareOKMsg() const {return (m_theMsgData.type == MsgType::PrepareOK);}
    bool IsPrepareMsg() const {return (m_theMsgData.type == MsgType::Prepare);}
    bool IsCommitMsg() const {return (m_theMsgData.type == MsgType::Commit);}
    
   
    std::shared_ptr<ClientMessage> GetClientMessagePayload() const;
    uint8_t * GetPayloadData(uint32_t& size) const;
   
    //ISerializable
    virtual const uint8_t *  Serialize(uint32_t &size) const override;
    virtual const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    virtual bool Deserialize() override ;
    
    void Print() const;
  private:
    void SetPayload(const ISerializable &payload);
    LogReplicationMsgData m_theMsgData;
   mutable std::vector<uint8_t> m_payload;
   mutable std::vector<uint8_t> m_serializedDataInclMsgAndPayload;
   const unsigned int MAX_PAYLOAD_SIZE = 1024;
};

#endif //CCM_LR_MESSAGE_H
