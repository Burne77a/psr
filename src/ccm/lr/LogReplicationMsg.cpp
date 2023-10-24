#include "LogReplicationMsg.h"
#include "Logger.h"

LogReplicationMsg::LogReplicationMsg(const MsgType typeOfMsg)
{
  m_theMsgData.type = typeOfMsg;
}

LogReplicationMsg::LogReplicationMsg(const MsgType typeOfMsg, const unsigned int opNumber, const unsigned int viewNumber, const unsigned int srcId, const unsigned int dstId)
{
  m_theMsgData.type = typeOfMsg;
  m_theMsgData.operationNumber = opNumber;
  m_theMsgData.viewNumber = viewNumber;
  m_theMsgData.srcId = srcId;
  m_theMsgData.dstId = dstId;
}




static const char * const msgTypeStr[3] =  {"Prepare","PrepareOK","Commit"};
const std::string LogReplicationMsg::GetMsgTypeAsString(const MsgType type)
{
  return std::string(msgTypeStr[(int)type]);
}

void LogReplicationMsg::Print() const
{
  LogMsg(LogPrioInfo,"LogReplicationMsg type: %d (%s) ViewNo: %u OpNumber: %u Src: %u Dst: %u " ,m_theMsgData.type,GetMsgTypeAsString(m_theMsgData.type).c_str(),
      m_theMsgData.viewNumber,m_theMsgData.operationNumber, m_theMsgData.srcId ,m_theMsgData.dstId);
}
