#include "UDPUniCastRcv.h"
#include "../NwAid.h"
#include "Logger.h"
#include "ErrorCodes.h"
#include <errnoLib.h>
#include <ioLib.h> 
#include <sys/socket.h>   



UDPUniCastRcv::UDPUniCastRcv()
{
  
}

UDPUniCastRcv::~UDPUniCastRcv()
{
  if(m_socket > 0)
  {
    close(m_socket);
    m_socket = -1;
  }
}

bool UDPUniCastRcv::Init(const int port)
{
  bool isInitSuccessfull = false;
  m_socket = NwAid::SetupRcvSocket(port);
  if(m_socket <= 0)
  {
    LogMsg(LogPrioCritical,"ERROR: UDPUniCastRcv::Init failed to setup socket %s %s %d", port);
    isInitSuccessfull = false;
  }
  else
  {
    LogMsg(LogPrioInfo,"UDPUniCastRcv::Init successfully created UDP socket %d. Listening to port %d",m_socket, port);
    isInitSuccessfull = true;
  }
  return isInitSuccessfull;
}




bool UDPUniCastRcv::Rcv(ISerializable & objToPopulate)
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
    LogMsg(LogPrioCritical,"ERROR: UDPUniCastRcv::Rcv failed %d", m_socket);
    isRcvSuccessfull = false;
  }
  return isRcvSuccessfull; 
}
