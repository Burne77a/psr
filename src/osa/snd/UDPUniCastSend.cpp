#include "UDPUniCastSend.h"
#include "Logger.h"
#include "ErrorCodes.h"
#include <errnoLib.h>
#include <ioLib.h> 
#include <sys/socket.h>   


UDPUniCastSend::UDPUniCastSend()
{
  m_dstSendInf.socket = -1;
}
UDPUniCastSend::~UDPUniCastSend()
{
  if(m_dstSendInf.socket > 0)
  {
    close(m_dstSendInf.socket);
    m_dstSendInf.socket = -1;
  }
}

bool UDPUniCastSend::Init(const std::string_view dstIp,const int port)
{
  if(!NwAid::SetupSendSocket(dstIp, port, m_dstSendInf))
  {
    LogMsg(LogPrioCritical,"ERROR: UDPUniCastSend::Init failed to create socket %d %s %d",m_dstSendInf.socket, dstIp.data(), port);
    m_dstSendInf.socket = -1;
    return false;
  }
  
  LogMsg(LogPrioInfo,"UDPUniCastSend::Init successfully created socket for UDP unicast to %d %s %d",m_dstSendInf.socket, dstIp.data(), port);
  return true;
}


bool UDPUniCastSend::Send(const ISerializable & objToSend)
{
  bool isSendSuccessfull = false;
  const OSAStatusCode sts = NwAid::Send(m_dstSendInf, objToSend);
  if(sts == OSA_OK)
  {
    isSendSuccessfull = true;
  }
  else if(sts == OSA_NO_DATA)
  {
    LogMsg(LogPrioCritical,"ERROR: UDPUniCastSend::Send failed - no data transmitted %d", m_dstSendInf.socket);
    isSendSuccessfull = false;
  }
  else
  {
    //todo: Better error handling.
    LogMsg(LogPrioCritical,"ERROR: UDPUniCastSend::Send failed %d", m_dstSendInf.socket);
    isSendSuccessfull = false;
  }
  return isSendSuccessfull; 
}
