#include "StateDataMsg.h"
#include "Logger.h"

StateDataMsg::StateDataMsg()
{
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_dataAndPayloadSerialized.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
}

StateDataMsg::StateDataMsg(const unsigned int appId,const MsgType type)
{
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_data.type = type;
  m_data.appId = appId;
  m_dataAndPayloadSerialized.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
}

StateDataMsg::StateDataMsg(const StateDataMsg &src) : m_data{src.m_data}
{
 //TODO: Ugly workaround. Make better later.
  m_dataAndPayloadSerialized.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
 std::memcpy(m_dataAndPayloadSerialized.data(),src.m_dataAndPayloadSerialized.data(),src.m_data.payloadSize + sizeof(m_data));
}

StateDataMsg::StateDataMsg(const MsgType type, const StateDataMsg &src) :  m_data{src.m_data}
{
  m_dataAndPayloadSerialized.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
  std::memcpy(m_dataAndPayloadSerialized.data(),src.m_dataAndPayloadSerialized.data(),src.m_data.payloadSize + sizeof(m_data));
  m_data.type = type;
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_dataAndPayloadSerialized.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
}
 
StateDataMsg::~StateDataMsg()
{
  
}

StateDataMsg& StateDataMsg::operator=(const StateDataMsg& other)
{
  if(this != &other)
  {
    m_data = other.m_data;
    m_dataAndPayloadSerialized.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
    std::memcpy(m_dataAndPayloadSerialized.data(),other.m_dataAndPayloadSerialized.data(),other.m_data.payloadSize + sizeof(m_data));
  }
  return *this;
}

uint8_t * StateDataMsg::GetPayloadBuffer(uint32_t &size) const
{
  size = m_data.payloadSize;
  return (m_dataAndPayloadSerialized.data() + sizeof(m_data));
}


void StateDataMsg::SetPayload(const ISerializable &objToSend)
{
  const uint8_t * pPayloadData = objToSend.Serialize(m_data.payloadSize);
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_data);
  const uint32_t sizeOfMeta = sizeof(m_data);
  m_dataAndPayloadSerialized.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
  if((pPayloadData != nullptr) && (m_data.payloadSize > 0))
  {
    m_dataAndPayloadSerialized.insert(m_dataAndPayloadSerialized.end(), pPayloadData, pPayloadData + m_data.payloadSize);
  }
}

uint8_t *  StateDataMsg::Serialize(uint32_t &size) const
{
  //TODO: adapt to endian differences, for now we just run little endian. 
  size = m_dataAndPayloadSerialized.size();
  return m_dataAndPayloadSerialized.data();
}

const uint8_t * StateDataMsg::GetSerializableDataBuffer(uint32_t &size) const
{
  m_dataAndPayloadSerialized.reserve(MAX_PAYLOAD_SIZE + sizeof(m_data));
  size = m_dataAndPayloadSerialized.capacity();
  return m_dataAndPayloadSerialized.data();
}

bool StateDataMsg::Deserialize()
{
  bool isSuccessfullyDeserialized = false;
  if((m_dataAndPayloadSerialized.data() != nullptr) &&  (m_dataAndPayloadSerialized.capacity() >= sizeof(m_data)))
  {
    m_data = *(StateData*)m_dataAndPayloadSerialized.data();
    isSuccessfullyDeserialized =  true;
  }
  else
  {
    LogMsg(LogPrioError,"ERROR:  StateDataMsg::Deserialize invalid buffers 0x%x %u %u",m_dataAndPayloadSerialized.data(),m_dataAndPayloadSerialized.capacity(), sizeof(m_data));
  }
  return isSuccessfullyDeserialized;
}

void StateDataMsg::Print() const
{
  LogMsg(LogPrioInfo,"StateDataMsg: PayloadSize: %u, Type: %d, appId: %u Capacity: %u Size: %u DataAddr: 0x%x,",m_data.payloadSize,m_data.type,m_data.appId,m_dataAndPayloadSerialized.capacity(),
      m_dataAndPayloadSerialized.size(),m_dataAndPayloadSerialized.data());
}


