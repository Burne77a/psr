#ifndef OSFUNCTIONS_UDPMCASTRCV_H_
#define OSFUNCTIONS_UDPMCASTRCV_H_
#include "../IReceiver.h"
#include <string_view>

class UDPMCastRcv : public IReceiver
{
  public:
    UDPMCastRcv();
    ~UDPMCastRcv();
    bool Init(const std::string_view group, const std::string_view ifIp,const int port);
    virtual bool Rcv(ISerializable & objToPopulate);

  private:
    int m_socket{-1};
};



#endif //OSFUNCTIONS_UDPMCASTRCV_H_
