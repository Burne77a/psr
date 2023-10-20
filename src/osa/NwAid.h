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
    typedef struct
    {
      int socket;
      struct sockaddr_in dstInf;
    }NwSendInf;
    
    static std::unique_ptr<IReceiver> CreateUniCastReceiver(const int port);
    static std::unique_ptr<ISender> CreateUniCastSender(const std::string_view dstIp, const int port);
    static bool SetupSendSocket(const std::string_view dstIp, const int dstPort, NwSendInf & nwSendInfToPopulate);
    static int SetupRcvSocket(const int port);
    static bool JoinMcastGroup(const std::string_view group, const std::string_view ifIp, const int socket);
    static bool SetOutgoingIfForMcastGroup(const std::string_view group, const std::string_view ifIp, const int socket);
    static OSAStatusCode Send(const NwSendInf &dstSendInf,const ISerializable & objToSend);
    static OSAStatusCode Rcv(const int socket, ISerializable & objToPopulte);
    static OSAStatusCode AddIpOnNwIf(int ifNo, const std::string_view ifIp);
    static OSAStatusCode RemoveIpOnNwIf(int ifNo, const std::string_view ifIp);
};


#endif //OSFUNCTIONS_NWAID_H_
