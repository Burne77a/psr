#include "UDPMCastSend.h"
#include "Logger.h"
#include "ErrorCodes.h"
#include <errnoLib.h>
#include <ioLib.h> 
#include <sys/socket.h>   


UDPMCastSend::UDPMCastSend()
{
  m_dstSendInf.socket = -1;
}
UDPMCastSend::~UDPMCastSend()
{
  if(m_dstSendInf.socket > 0)
  {
    close(m_dstSendInf.socket);
    m_dstSendInf.socket = -1;
  }
}

bool UDPMCastSend::Init(const std::string_view group, const std::string_view ifIp,const int port)
{
  if(!NwAid::SetupSendSocket(group, port, m_dstSendInf))
  {
    LogMsg(LogPrioCritical,"ERROR: UDPMCastSend::Init failed to create socket %d %s %s %d",m_dstSendInf.socket, group.data(), ifIp.data(), port);
    m_dstSendInf.socket = -1;
    return false;
  }
  
  if(!NwAid::SetOutgoingIfForMcastGroup(group, ifIp, m_dstSendInf.socket))
  {
    LogMsg(LogPrioCritical,"ERROR: UDPMCastSend::Init failed to assign if for group %d %s %s %d",m_dstSendInf.socket, group.data(), ifIp.data(), port);
    return false;
  }
  LogMsg(LogPrioInfo,"UDPMCastSend::Init successfully created socket and assigned if for group %d %s %s %d",m_dstSendInf.socket, group.data(), ifIp.data(), port);
  return true;
}


bool UDPMCastSend::Send(const ISerializable & objToSend)
{
  bool isSendSuccessfull = false;
  const OSAStatusCode sts = NwAid::Send(m_dstSendInf, objToSend);
  if(sts == OSA_OK)
  {
    isSendSuccessfull = true;
  }
  else if(sts == OSA_NO_DATA)
  {
    LogMsg(LogPrioCritical,"ERROR: UDPMCastSend::Send failed - no data transmitted %d", m_dstSendInf.socket);
    isSendSuccessfull = false;
  }
  else
  {
    //todo: Better error handling.
    LogMsg(LogPrioCritical,"ERROR: UDPMCastSend::Send failed %d", m_dstSendInf.socket);
    isSendSuccessfull = false;
  }
  return isSendSuccessfull; 
}
