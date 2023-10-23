#ifndef OSFUNCTIONS_NWAID_H_
#define OSFUNCTIONS_NWAID_H_

#include "ISerializable.h"
#include "ErrorCodes.h"
#include "ISender.h"
#include "IReceiver.h"
#include <string_view>
#include <netinet/in.h>
#include <memory>

class NwAid
{
  public:
    
    enum class EthIfNo
    {
      IfOne = 0,
      IfTwo
    }; 
    typedef struct
    {
      int socket;
      struct sockaddr_in dstInf;
      std::string ipAdr;
    }NwSendInf;
    
    static std::unique_ptr<IReceiver> CreateUniCastReceiver(const int port);
    static std::unique_ptr<ISender> CreateUniCastSender(const std::string_view dstIp, const int port);
    static bool SetupSendSocket(const std::string_view dstIp, const int dstPort, NwSendInf & nwSendInfToPopulate);
    static int SetupRcvSocket(const int port);
    static bool JoinMcastGroup(const std::string_view group, const std::string_view ifIp, const int socket);
    static bool SetOutgoingIfForMcastGroup(const std::string_view group, const std::string_view ifIp, const int socket);
    static OSAStatusCode Send(const NwSendInf &dstSendInf,const ISerializable & objToSend);
    static OSAStatusCode Rcv(const int socket, ISerializable & objToPopulte);
    static OSAStatusCode AddIpOnNwIf(const EthIfNo ifNo, const std::string_view ifIp);
    static OSAStatusCode RemoveIpOnNwIf(const EthIfNo ifNo, const std::string_view ifIp);
    static void ArpFlush();
    static const std::string GetIfNameAsStr(const EthIfNo type);
};


#endif //OSFUNCTIONS_NWAID_H_
