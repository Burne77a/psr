#ifndef CCM_HEARBEAT_H
#define CCM_HEARBEAT_H

#include "ISerializable.h"

class HeartbeatCCM : public ISerializable
{
public:
    HeartbeatCCM();
    ~HeartbeatCCM();
    
    bool Serialize(uint8_t *& pSerializedData, uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
};

#endif //CCM_HEARBEAT_H
