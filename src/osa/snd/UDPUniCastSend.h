#ifndef OSFUNCTIONS_UDPUNICASTSEND_H_
#define OSFUNCTIONS_UDPUNICASTSEND_H_

#include "../ISender.h"
#include "../NwAid.h"

class UDPUniCastSend : public ISender
{
  public:
    UDPUniCastSend();
    ~UDPUniCastSend();
    bool Init(const std::string_view dstIp, const int port);
    bool Send(const ISerializable & objToSend) override;
    std::string GetIpAddr(void) override {return m_dstSendInf.ipAdr;}

  private:
    NwAid::NwSendInf m_dstSendInf;
};

#endif //OSFUNCTIONS_UDPUNICASTRCV_H_
