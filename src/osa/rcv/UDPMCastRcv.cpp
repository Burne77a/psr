#include "UDPMCastRcv.h"
#include "../NwAid.h"
#include "Logger.h"
#include "ErrorCodes.h"
#include <errnoLib.h>
#include <ioLib.h> 
#include <sys/socket.h>   



UDPMCastRcv::UDPMCastRcv()
{
  
}

UDPMCastRcv::~UDPMCastRcv()
{
  if(m_socket > 0)
  {
    close(m_socket);
    m_socket = -1;
  }
}

bool UDPMCastRcv::Init(const std::string_view group, const std::string_view ifIp, const int port)
{
  bool isInitSuccessfull = false;
  m_socket = NwAid::SetupRcvSocket(port);
  if(m_socket <= 0)
  {
    LogMsg(LogPrioCritical,"ERROR: UDPMCastRcv::Init to setup socket %s %s %d", group.data(), ifIp.data(), port);
    isInitSuccessfull = false;
  }
  else
  {
    if(NwAid::JoinMcastGroup(group, ifIp, m_socket))
    {
      LogMsg(LogPrioInfo,"UDPMCastRcv::Init successfully created socket and joined group %d %s %s %d",m_socket, group.data(), ifIp.data(), port);
      isInitSuccessfull = true;
    }
    else
    {
      LogMsg(LogPrioCritical,"ERROR: UDPMCastRcv::Init failed to join mcast group %s %s %d", group.data(), ifIp.data(), port);
      close(m_socket);
      m_socket = -1;
    }
  }
  return isInitSuccessfull;
}




bool UDPMCastRcv::Rcv(ISerializable & objToPopulate)
{
  bool isRcvSuccessfull = false;
  
  const OSAStatusCode sts = NwAid::Rcv(m_socket, objToPopulate);
  if(sts == OSA_OK)
  {
    isRcvSuccessfull = true;
  }
  else if(sts == OSA_NO_DATA)
  {
    isRcvSuccessfull = false;
  }
  else
  {
    //todo: Better error handling.
    LogMsg(LogPrioCritical,"ERROR: UDPMCastRcv::Rcv failed %d", m_socket);
    isRcvSuccessfull = false;
  }
  return isRcvSuccessfull; 
}
