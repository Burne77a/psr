#ifndef OSFUNCTIONS_UDPUNICASTRCV_H_
#define OSFUNCTIONS_UDPUNICASTRCV_H_
#include "../IReceiver.h"

class UDPUniCastRcv : public IReceiver
{
  public:
    UDPUniCastRcv();
    ~UDPUniCastRcv();
    bool Init(const int port);
    virtual bool Rcv(ISerializable & objToPopulate);

  private:
    int m_socket{-1};
};


#endif //OSFUNCTIONS_UDPUNICASTRCV_H_
