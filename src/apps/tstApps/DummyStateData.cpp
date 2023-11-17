#include "DummyStateData.h"
#include "Logger.h"

DummyStateData::DummyStateData(const unsigned int payloadSize)
{
  m_data.payloadSize = payloadSize;
  for(unsigned int i = 0; i < payloadSize; i++ )
  {
    m_payload.push_back((uint8_t)i);
  }
}

DummyStateData::DummyStateData()
{
  m_data.payloadSize = 0U;
}

DummyStateData::~DummyStateData()
{
  
}

uint8_t *  DummyStateData::Serialize(uint32_t &size) const
{
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_serializedData.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
  if((m_payload.data() != nullptr) && (m_data.payloadSize > 0))
  {
    m_serializedData.insert(m_serializedData.end(), m_payload.data(), m_payload.data() + m_data.payloadSize);
  }
  
  
  //TODO: adapt to endian differences, for now we just run little endian. 
  size = m_serializedData.size();
  return m_serializedData.data();
}

const uint8_t * DummyStateData::GetSerializableDataBuffer(uint32_t &size) const
{
  m_serializedData.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
  size = m_serializedData.capacity();
  return m_serializedData.data();
}

bool DummyStateData::Deserialize()
{
  m_data = *(TheData*)m_serializedData.data();
  if(m_data.payloadSize > 0)
  {
    uint8_t * pPayloadStart = m_serializedData.data() + sizeof(m_data);
    
    m_payload.assign(pPayloadStart, pPayloadStart + m_data.payloadSize);
  }
  return true;
}

void DummyStateData::Print() const
{
  LogMsg(LogPrioInfo,"DummyStateData: SeqNr %u Size: %u, postPatern: %s serializedSize: %d payloadSize: %d",m_data.seqNr, m_data.payloadSize,m_data.postPattern,m_serializedData.size(),m_payload.size());
}

