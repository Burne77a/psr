#ifndef CCM_LE_MESSAGE_H
#define CCM_LE_MESSAGE_H
#include "ISerializable.h"
#include <string>

class LeaderElectionMsg : public ISerializable
{
  public:
  enum class MsgType
  {
    ElectionStart = 0,
    ElectionCompleted,
    ElectionVote
  };   
  private:
    struct LeaderElectionMsgData
    {
      MsgType type{MsgType::ElectionStart};
      unsigned int viewNumber{0U};
      unsigned int opNumber{0U};
      unsigned int senderId{0U};
      unsigned int idOfVoteReceiver{0U};
    };
  public:
    LeaderElectionMsg(const MsgType typeOfMsg);
    ~LeaderElectionMsg() = default;
    MsgType GetType() const {return m_theMsgData.type;} 
    static const std::string GetMsgTypeAsString(const MsgType type);
    unsigned int GetViewNumber() const {return m_theMsgData.viewNumber;} 
    unsigned int GetSenderId() const {return m_theMsgData.senderId;}
    unsigned int GetIdOfVoteDst() const {return m_theMsgData.idOfVoteReceiver;}
    unsigned int GetOpNumber() const {return m_theMsgData.opNumber;}
    
    void SetViewNumber(const unsigned int viewNumber){m_theMsgData.viewNumber = viewNumber;}
    void SetSenderId(const unsigned int senderId){m_theMsgData.senderId = senderId;}
    void SetVoteDstId(const unsigned int voteDstId){m_theMsgData.idOfVoteReceiver = voteDstId;}
    void SetOpNumber(const unsigned int opNumber){m_theMsgData.opNumber = opNumber;}
    
    
    
    void SetViewNumberWhenRcvd(const unsigned int viewNo){m_viewNumberWhenRcvd = viewNo;}
    unsigned int GetViewNumberWhenRcvd() const {return m_viewNumberWhenRcvd;}
    static bool IsValidViewNumber(const unsigned int viewNumber) {return viewNumber > 0;}
    bool IsMsgViewNumberHigherThanViewNumberWhenRcvd() const
    { 
      return IsValidViewNumber(m_viewNumberWhenRcvd) && IsValidViewNumber(m_theMsgData.viewNumber) && (m_viewNumberWhenRcvd < m_theMsgData.viewNumber);
    } 
    
    bool IsElectionStartMsg() {return m_theMsgData.type == MsgType::ElectionStart;}
    bool IsVoteMsg(){return m_theMsgData.type == MsgType::ElectionVote;}
    bool IsElectionCompletedMsg() {return m_theMsgData.type == MsgType::ElectionCompleted;}
    
    //ISerializable
    virtual const uint8_t *  Serialize(uint32_t &size) const override;
    virtual const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    virtual bool Deserialize() override ;
    
    void Print() const;
  private:
    LeaderElectionMsgData m_theMsgData;
    unsigned int m_viewNumberWhenRcvd{0U};
};


#endif //CCM_LE_MESSAGE_H
