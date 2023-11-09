#include "HeartbeatAFD.h"
#include "Logger.h"

HeartbeatAFD::HeartbeatAFD()
{
  InitHeartbeatData(m_dataToExchange);
}

HeartbeatAFD::~HeartbeatAFD()
{
  
}

uint8_t *  HeartbeatAFD::Serialize(uint32_t &size) const
{
  //TODO: adapt to endian differences, for now we just run little endian. 
  size = sizeof(m_dataToExchange);
  return (uint8_t*)&m_dataToExchange;
}

const uint8_t * HeartbeatAFD::GetSerializableDataBuffer(uint32_t &size) const
{
  size = sizeof(m_dataToExchange);
  return (uint8_t*)&m_dataToExchange;
}

bool HeartbeatAFD::Deserialize()
{
  //TODO: adapt to endian differences, for now we just run little endian. 
  return true;
}

void HeartbeatAFD::InitHeartbeatData(HeartbeatData &hbData)
{
  memset((void*)&hbData,0, sizeof(HeartbeatData));
}
