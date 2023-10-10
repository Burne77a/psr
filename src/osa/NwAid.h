#ifndef OSFUNCTIONS_NWAID_H_
#define OSFUNCTIONS_NWAID_H_

#include "ISerializable.h"
#include "ErrorCodes.h"
#include <string_view>
#include <netinet/in.h> 

class NwAid
{
  public:
    typedef struct
    {
      int socket;
      struct sockaddr_in dstInf;
    }NwSendInf;
    
    static bool SetupSendSocket(const std::string_view dstIp, const int dstPort, NwSendInf & nwSendInfToPopulate);
    static int SetupRcvSocket(const int port);
    static bool JoinMcastGroup(const std::string_view group, const std::string_view ifIp, const int socket);
    static bool SetOutgoingIfForMcastGroup(const std::string_view group, const std::string_view ifIp, const int socket);
    static OSAStatusCode Send(const NwSendInf &dstSendInf,const ISerializable & objToSend);
    static OSAStatusCode Rcv(const int socket, ISerializable & objToPopulte);
};


#endif //OSFUNCTIONS_NWAID_H_
