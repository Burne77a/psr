#ifndef OSFUNCTIONS_ISER_H_
#define OSFUNCTIONS_ISER_H_

#include <vector>
#include <cstdint>  

class ISerializable {
public:
    virtual ~ISerializable() = default;
    virtual std::vector<uint8_t> Serialize() const = 0;
    virtual void Deserialize(const std::vector<uint8_t>& data) = 0;
};

#endif //#define OSFUNCTIONS_ISER_H_
