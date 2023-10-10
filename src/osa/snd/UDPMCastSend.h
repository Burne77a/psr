#ifndef OSFUNCTIONS_UDPMCASTSEND_H_
#define OSFUNCTIONS_UDPMCASTSEND_H_

#include "../ISender.h"
#include "../NwAid.h"

class UDPMCastSend : public ISender
{
  public:
    UDPMCastSend();
    ~UDPMCastSend();
    bool Init(const std::string_view group, const std::string_view ifIp,const int port);
    virtual bool Send(const ISerializable & objToSend);

  private:
    NwAid::NwSendInf m_dstSendInf;
};


#endif //OSFUNCTIONS_UDPMCASTSEND_H_
