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
const std::string LeaderElectionMsg::GetMsgTypeStateAsString(const MsgType type)
{
  return std::string(msgTypeStr[(int)type]);
}

void LeaderElectionMsg::Print() const
{
  LogMsg(LogPrioInfo,"LeaderElectionMsg type: %d (%s) ViewNo: %u (%u) Sender: %u Vote: %u",m_theMsgData.type,GetMsgTypeStateAsString(m_theMsgData.type).c_str(),
      m_viewNumberWhenRcvd,m_theMsgData.viewNumber, m_theMsgData.senderId,m_theMsgData.idOfVoteReceiver);
}
