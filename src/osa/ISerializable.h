#ifndef OSFUNCTIONS_ISER_H_
#define OSFUNCTIONS_ISER_H_

#include <vector>
#include <cstdint>  

class ISerializable 
{
public:
    virtual ~ISerializable() = default;
    virtual const uint8_t *  Serialize(uint32_t &size) const = 0;
    virtual const uint8_t * GetSerializableDataBuffer(uint32_t &size) const = 0;
    virtual bool Deserialize() = 0;
};

#endif //#define OSFUNCTIONS_ISER_H_
