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
    bool Send(const ISerializable & objToSend) override;
    std::string GetIpAddr(void) override {return m_dstSendInf.ipAdr;}

  private:
    NwAid::NwSendInf m_dstSendInf;
};


#endif //OSFUNCTIONS_UDPMCASTSEND_H_
