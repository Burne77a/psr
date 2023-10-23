#ifndef CCM_LR_MESSAGE_H
#define CCM_LR_MESSAGE_H
#include "ISerializable.h"
#include <string>

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
    LogReplicationMsg(const MsgType typeOfMsg);
    ~LogReplicationMsg() = default;
    MsgType GetType() const {return m_theMsgData.type;} 
    static const std::string GetMsgTypeAsString(const MsgType type);
    
    unsigned int GetViewNumber() const {return m_theMsgData.viewNumber;} 
    unsigned int GetSrcId() const {return m_theMsgData.srcId;}
    unsigned int GetDstId() const {return m_theMsgData.dstId;}
    unsigned int GetOpNumber() const {return m_theMsgData.operationNumber;}
    
    void SetViewNumber(const unsigned int viewNumber){m_theMsgData.viewNumber = viewNumber;}
    void SetSrcId(const unsigned int srcId){m_theMsgData.srcId = srcId;}
    void SetDstId(const unsigned int dstId){m_theMsgData.dstId = dstId;}
    void SetOpNumber(const unsigned int opNumber){m_theMsgData.operationNumber = opNumber;}
    
    
    
   
    //ISerializable
    virtual const uint8_t *  Serialize(uint32_t &size) const override;
    virtual const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    virtual bool Deserialize() override ;
    
    void Print() const;
  private:
    LogReplicationMsgData m_theMsgData;
};

#endif //CCM_LR_MESSAGE_H
