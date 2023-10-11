#include "HeartbeatCCM.h"

HeartbeatCCM::HeartbeatCCM()
{
  
}

HeartbeatCCM::~HeartbeatCCM()
{
  
}

bool HeartbeatCCM::Serialize(uint8_t *& pSerializedData, uint32_t &size) const
{
  return false;
}

const uint8_t * HeartbeatCCM::GetSerializableDataBuffer(uint32_t &size) const
{
  return nullptr;
}

bool HeartbeatCCM::Deserialize()
{
  return false;
}
