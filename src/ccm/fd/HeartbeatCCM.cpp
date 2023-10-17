#include "HeartbeatCCM.h"
#include "Logger.h"

HeartbeatCCM::HeartbeatCCM()
{
  InitHeartbeatData(m_dataToExchange);
}

HeartbeatCCM::~HeartbeatCCM()
{
  
}

uint8_t *  HeartbeatCCM::Serialize(uint32_t &size) const
{
  m_dataToExchange.m_connectionPerceptionBits = m_connectionPerception.to_ulong();
  //TODO: adapt to endian differences, for now we just run little endian. 
  size = sizeof(m_dataToExchange);
  return (uint8_t*)&m_dataToExchange;
}

const uint8_t * HeartbeatCCM::GetSerializableDataBuffer(uint32_t &size) const
{
  size = sizeof(m_dataToExchange);
  return (uint8_t*)&m_dataToExchange;
}

bool HeartbeatCCM::Deserialize()
{
  //TODO: adapt to endian differences, for now we just run little endian. 
  m_connectionPerception = m_dataToExchange.m_connectionPerceptionBits;
  return true;
}

void HeartbeatCCM::InitHeartbeatData(HeartbeatData &hbData)
{
  memset((void*)&hbData,0, sizeof(HeartbeatData));
}
