#include "NwAid.h"
#include "snd/UDPUniCastSend.h"
#include "rcv/UDPUniCastRcv.h"
#include "Logger.h"
#include "ErrorCodes.h"
#include <arpa/inet.h>
#include <errnoLib.h>
#include <ioLib.h> 
#include <sys/types.h>    
#include <sys/socket.h>   
#include <netinet/in.h>
#include <selectLib.h>


std::unique_ptr<IReceiver> NwAid::CreateUniCastReceiver(const int port)
{
  std::unique_ptr<UDPUniCastRcv> pRcv = std::make_unique<UDPUniCastRcv>();
  if(pRcv != nullptr)
  {
    if(pRcv->Init(port))
    {
      return pRcv;
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: NwAid::CreateUniCastReceiver init socket %d 0x%x (%s)",port, errnoGet(), strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: NwAid::CreateUniCastReceiver Failed to create unique pointer %d 0x%x (%s)",port, errnoGet(), strerror(errnoGet()));
  }
  return pRcv;
}

std::unique_ptr<ISender> NwAid::CreateUniCastSender(const std::string_view dstIp, const int port)
{
  std::unique_ptr<UDPUniCastSend> pSend = std::make_unique<UDPUniCastSend>();
  if(pSend != nullptr)
  {
    if(pSend->Init(dstIp, port))
    {
      return pSend;
    }
    else
    {
      LogMsg(LogPrioCritical, "ERROR: NwAid::CreateUniCastSender init socket %s:%d 0x%x (%s)",dstIp.data(), port, errnoGet(), strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioCritical, "ERROR: NwAid::CreateUniCastSender Failed to create unique pointer %s:%d 0x%x (%s)",dstIp.data(), port, errnoGet(), strerror(errnoGet()));
  }
  return pSend;
}

bool NwAid::SetupSendSocket(const std::string_view dstIp, const int dstPort, NwSendInf & nwSendInfToPopulate)
{
  nwSendInfToPopulate.socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (nwSendInfToPopulate.socket <= 0)
  {
    LogMsg(LogPrioCritical, "ERROR: NwAid::SetupSendSocket Failed to create send socket %s:%d 0x%x (%s)",dstIp.data(),dstPort, errnoGet(),strerror(errnoGet()));
    return false;
  }

  memset(&nwSendInfToPopulate.dstInf, 0, sizeof(nwSendInfToPopulate.dstInf));
  nwSendInfToPopulate.dstInf.sin_family = AF_INET;
  inet_pton(AF_INET, dstIp.data(),  &nwSendInfToPopulate.dstInf.sin_addr.s_addr);
  nwSendInfToPopulate.dstInf.sin_port = htons(dstPort);
  
  const BOOL on = TRUE;
  const STATUS status = ioctl (nwSendInfToPopulate.socket, FIONBIO, &on);
  if(status != OK)
  {
    LogMsg(LogPrioCritical,"ERROR: NwAid::SetupSendSocket failed to set to non-blocking %d %s:%d 0x%x (%s)",status, dstIp.data(),dstPort, errnoGet(),strerror(errnoGet()));
    close(nwSendInfToPopulate.socket);
    nwSendInfToPopulate.socket = -1;
    return false;
  }
  
  return true;
}




int NwAid::SetupRcvSocket(const int port)
{
  struct sockaddr_in address;
  const int theSocket = socket(AF_INET, SOCK_DGRAM, 0);
  int opt = 1;
  if (theSocket <= 0)
  {
    LogMsg(LogPrioCritical,"ERROR: SetupRcvSocket failed to create socket");
    return OSA_SOCKET_ERROR;
  }
    
    
  if (setsockopt(theSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt)) != OK)
  {
    LogMsg(LogPrioCritical,"ERROR: SetupRcvSocket setsockopt failed");
    close(theSocket);
    return OSA_SOCKET_ERROR;
  }
  
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);
  if (bind(theSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
  {
    LogMsg(LogPrioCritical,"ERROR: SetupRcvSocket bind failed0x%x (%s)",errnoGet(),strerror(errnoGet()));
    close(theSocket);
    return OSA_SOCKET_ERROR;
  }
  
  const BOOL on = TRUE;
  const STATUS status = ioctl (theSocket, FIONBIO, &on);
  if(status != OK)
  {
    LogMsg(LogPrioCritical,"ERROR: SetupRcvSocket failed to set to non-blocking status: %d 0x%x (%s)",status,errnoGet(),strerror(errnoGet()));
    close(theSocket);
    return OSA_SOCKET_ERROR;
  }
  
  return theSocket;
}


bool NwAid::JoinMcastGroup(const std::string_view group, const std::string_view ifIp, const int socket)
{
  bool isJoinSuccessfull = false;
  struct ip_mreq mreq;
   
  inet_pton(AF_INET, group.data(), (void*)&mreq.imr_multiaddr);
  inet_pton(AF_INET, ifIp.data(), (void*) &mreq.imr_interface);
  if (setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) != OK)
  {
    LogMsg(LogPrioCritical,"JoinMcastGroup failed to join mcast %s on interface %s 0x%x (%s)",group.data(),ifIp.data(),errnoGet(),strerror(errnoGet()));
    isJoinSuccessfull = false;
  }
  else
  {
    isJoinSuccessfull = true;
    LogMsg(LogPrioInfo,"JoinMcastGroup joined mcast %s on interface %s 0x%x ", group.data(),ifIp.data());
  }
  return isJoinSuccessfull;
}

bool NwAid::SetOutgoingIfForMcastGroup(const std::string_view group, const std::string_view ifIp, const int socket)
{
  struct ip_mreq mreq;
    
  inet_pton(AF_INET, group.data(), (void*)&mreq.imr_multiaddr.s_addr);
  inet_pton(AF_INET, ifIp.data(), (void*) &mreq.imr_interface.s_addr);
  
  //Set default outgoing interface for this socket and multicast traffic
  if(setsockopt (socket, IPPROTO_IP, IP_MULTICAST_IF, (char *)&mreq.imr_interface.s_addr, sizeof (mreq.imr_interface.s_addr)) != OK)
  {
    LogMsg(LogPrioCritical,"ERROR: NwAid::SetOutgoingIfForMcastGroup failed to set default if for mcast %s on interface %s errno: 0x%x (%s) socket %d ", group.data(),ifIp.data(),errnoGet(),strerror(errnoGet()),socket);
    return false;
  }
  else
  {
    LogMsg(LogPrioInfo,"strerror(errnoGet()) set interface %s as default for mcast for socket %d", ifIp.data(), socket);
  }
  return true;
  
}

OSAStatusCode NwAid::Send(const NwSendInf &dstSendInf,const ISerializable & objToSend)
{
  uint32_t sizeSerializedData = 0;
  const void * const pSerializedDataToSend = objToSend.Serialize(sizeSerializedData);
  
  if((pSerializedDataToSend == NULL) || (sizeSerializedData == 0))
  {
    LogMsg(LogPrioCritical,"ERROR: NwAid::Send No data to send. objToSend return null ptr or zero size data 0x%x %u ", pSerializedDataToSend,sizeSerializedData);
    return OSA_ERROR;
  }
  
  const int sndSts = sendto(dstSendInf.socket,pSerializedDataToSend, sizeSerializedData, 0,(struct sockaddr *) &dstSendInf.dstInf, 
      sizeof (dstSendInf.dstInf));
  
  if(sndSts < 0)
  {
    LogMsg(LogPrioCritical,"ERROR: NwAid::Send Failed to send. %d %u 0x%x %s", sndSts,sizeSerializedData,errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;   
  }
  else if (sndSts != (int)sizeSerializedData)     
  {    
    LogMsg(LogPrioCritical,"ERROR: NwAid::Send Failed to send all data. %d %u 0x%x %s", sndSts,sizeSerializedData,errnoGet(),strerror(errnoGet()));
    return OSA_ERROR;   
  }
  
  return OSA_OK; 
}


OSAStatusCode NwAid::Rcv(const int socket, ISerializable & objToPopulte)
{
  uint32_t size = 0;
  const uint8_t* const pBuffer = objToPopulte.GetSerializableDataBuffer(size);
  if((pBuffer == NULL) || (size == 0))
  {
    LogMsg(LogPrioCritical,"ERROR: NwAid::Rcv ISerializable return invalid buffer data %d 0x%x %u",socket,pBuffer,size);
    return OSA_ERROR;
  }
  
  
  const int recvSize = recv(socket, (char*)pBuffer, size, 0);
  if(recvSize == size)
  {
    if(objToPopulte.Deserialize())
    {    
      return OSA_OK;
    }
    else
    {
      LogMsg(LogPrioCritical,"ERROR: NwAid::Rcv ISerializable failed to deserialize new data %d 0x%x %u",socket,pBuffer,size);
      return OSA_ERROR;
    }
  }
  else if(recvSize == 0)
  {
    return OSA_NO_DATA;
  }
  else if(recvSize < 0)
  {
    if(EWOULDBLOCK != errnoGet())
    {
      LogMsg(LogPrioCritical,"ERROR: NwAid::Rcv recv return error %d 0x%x (%s) 0x%x %u",recvSize,errnoGet(),strerror(errnoGet()), pBuffer,size);
      return OSA_ERROR;
    }
    else
    {
      return OSA_NO_DATA;
    }
  }
  else
  {
    LogMsg(LogPrioCritical,"ERROR: NwAid::Rcv Size missmatch %d %d 0x%x (%s) 0x%x %u",recvSize,size,errnoGet(),strerror(errnoGet()), pBuffer,size);
    return OSA_ERROR;
  }
}
