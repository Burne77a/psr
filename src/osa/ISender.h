#ifndef OSFUNCTIONS_ISENDER_H_
#define OSFUNCTIONS_ISENDER_H_

#include "ISerializable.h"

class ISender {
public:
    virtual ~ISender() = default;
    virtual bool Send(const ISerializable & objToSend) = 0;
};

#endif //OSFUNCTIONS_ISENDER_H_
