#ifndef OSFUNCTIONS_IRECV_H_
#define OSFUNCTIONS_IRECV_H_

#include "ISerializable.h"

class IReceiver {
public:
    virtual ~IReceiver() = default;
    virtual bool Rcv(ISerializable & objToPopulate) = 0;
};

#endif //OSFUNCTIONS_IRECV_H_
