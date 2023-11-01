#include "LeaderElectionMsg.h"
#include "Logger.h"

LeaderElectionMsg::LeaderElectionMsg(const MsgType typeOfMsg)
{
  m_theMsgData.type = typeOfMsg;
}

const uint8_t *  LeaderElectionMsg::Serialize(uint32_t &size) const
{
  size = sizeof(m_theMsgData);
  return (uint8_t*) &m_theMsgData;
}

const uint8_t * LeaderElectionMsg::GetSerializableDataBuffer(uint32_t &size) const
{
  size = sizeof(m_theMsgData);
  return (uint8_t*) &m_theMsgData;
}

bool LeaderElectionMsg::Deserialize()
{
  //Nothing needs to be done.
  return true;
}

static const char * const msgTypeStr[3] =  {"ElectionStart","ElectionCompleted","ElectionVote"};
const std::string LeaderElectionMsg::GetMsgTypeAsString(const MsgType type)
{
  return std::string(msgTypeStr[(int)type]);
}

void LeaderElectionMsg::Print() const
{
  LogMsg(LogPrioInfo,"LeaderElectionMsg type: %d (%s) ViewNo:OpNumber: %u:%u (%u) Sender: %u Vote: %u",m_theMsgData.type,GetMsgTypeAsString(m_theMsgData.type).c_str(),
      m_theMsgData.viewNumber,m_theMsgData.opNumber,m_viewNumberWhenRcvd, m_theMsgData.senderId,m_theMsgData.idOfVoteReceiver);
}
