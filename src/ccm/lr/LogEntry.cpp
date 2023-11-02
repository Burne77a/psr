#include "LogEntry.h"
#include "Logger.h"
#include <errnoLib.h> 


std::unique_ptr<LogEntry> LogEntry::CreateEntry(const LogReplicationMsg &msg)
{
  std::shared_ptr<ClientMessage> pCm = msg.GetClientMessagePayload();
  ClientRequestId rid{};
  if(pCm)
  {
    rid = pCm->GetReqId();
  }
  std::unique_ptr<LogEntry> pNewEntry = std::make_unique<LogEntry>(msg.GetOpNumber(),msg.GetViewNumber(),rid,msg.GetClientMessagePayload());
  if(!pNewEntry)
  {
    LogMsg(LogPrioCritical, "ERROR LogEntry::CreateEntry failed to LogEntry. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
  return pNewEntry;
}

std::unique_ptr<LogEntry> LogEntry::CreateEntry(const SyncMsg &msg)
{
   std::unique_ptr<LogEntry> pNewEntry = std::make_unique<LogEntry>();
  if(!pNewEntry)
  {
    LogMsg(LogPrioCritical, "ERROR LogEntry::CreateEntry failed to LogEntry. Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
    msg.Print();
  }
  else
  {
    pNewEntry->PopulateMeFrom(msg);
  }
  return pNewEntry;
}


LogEntry::LogEntry(const unsigned int opNumber, const unsigned int viewNumber, ClientRequestId &rid, std::shared_ptr<ClientMessage> pClientMsg) :
m_pClientMsg{pClientMsg}
{
  m_entryData.opNumber = opNumber;
  m_entryData.viewNumber = viewNumber;
  m_entryData.rid = rid;
  m_entryData.payloadSize = 0; 
  if(m_pClientMsg)
  {
    (void)m_pClientMsg->GetSerializableDataBuffer( m_entryData.payloadSize);
  }
}

LogEntry::LogEntry()
{
  
}

const uint8_t * LogEntry::Serialize(uint32_t &size) const 
{
  const uint8_t * pStartOfMeta = (uint8_t*)(&m_entryData);
  const uint32_t sizeOfMeta = sizeof(m_entryData);
  m_serializedDataInclPayload.assign(pStartOfMeta, pStartOfMeta + sizeOfMeta);
  
  
  if(m_pClientMsg)
  {
   uint32_t payloadSize = 0U;
   const uint8_t *pPayloadBufferStart = m_pClientMsg->Serialize(payloadSize);
   if(pPayloadBufferStart != nullptr && payloadSize > 0)
   {
     if(payloadSize <= MAX_PAYLOAD_SIZE)
     {
       m_serializedDataInclPayload.insert(m_serializedDataInclPayload.end(), pPayloadBufferStart, pPayloadBufferStart + payloadSize);
     }
     else
     {
       LogMsg(LogPrioError, "ERROR: LogEntry::Serialize %u, Req: %s to large payload size 0x%x %u",m_entryData.opNumber,m_entryData.rid.GetIdAsStr().c_str(),pPayloadBufferStart,payloadSize);
     }
  
   }
   else
   {
     LogMsg(LogPrioError, "ERROR: LogEntry::Serialize %u, Req: %s invalid payload ptr and/or size 0x%x %u",m_entryData.opNumber,m_entryData.rid.GetIdAsStr().c_str(),pPayloadBufferStart,payloadSize);
   }
  }
 
  size = m_serializedDataInclPayload.size();
  return m_serializedDataInclPayload.data();
}

const uint8_t * LogEntry::GetSerializableDataBuffer(uint32_t &size) const 
{
  m_serializedDataInclPayload.reserve(MAX_PAYLOAD_SIZE + sizeof(m_entryData));
  size = m_serializedDataInclPayload.capacity();
  return m_serializedDataInclPayload.data();
}

bool LogEntry::Deserialize()
{
  bool isSuccessfullyDeserialized = false;
  LogEntryData  * pEntryData = (LogEntryData *)m_serializedDataInclPayload.data();
  if(pEntryData != nullptr && m_serializedDataInclPayload.capacity() >= sizeof(LogEntryData))
  {
    m_entryData = *pEntryData;
    const uint32_t payloadSize = m_entryData.payloadSize; 
    if(payloadSize > 0)
    {
      if(!m_pClientMsg)
      {
        const uint8_t *pPayloadStart = m_serializedDataInclPayload.data() + sizeof(LogEntryData);
        m_pClientMsg = ClientMessage::CreateClientMessage(pPayloadStart,payloadSize);
        if(m_pClientMsg)
        {
          isSuccessfullyDeserialized = true;
        }
        else
        {
          LogMsg(LogPrioError,"LogEntry::Deserialize Failed to create ClientMessage from payload ");
        }
              
      }
      else
      {
        LogMsg(LogPrioError,"LogEntry::Deserialize Existing clientMessage. Aborting ");
      }
      
    }
    else
    {
      LogMsg(LogPrioError,"LogEntry::Deserialize No payload - fishy... We always assume a ClientMessage ");
    }
  }
  else
  {
    LogMsg(LogPrioError,"LogEntry::Deserialize invalid incoming data 0x%x, %u",pEntryData, m_serializedDataInclPayload.size());
  }
  return isSuccessfullyDeserialized;
}

void LogEntry::PopulateMeFrom(const SyncMsg &msg)
{
  uint32_t entryDataSize = 0U;
  uint8_t * pEntryData = msg.GetDeserializedPayload(entryDataSize);

  if((pEntryData != nullptr) && (entryDataSize >= sizeof(LogEntryData)))
  { 
    uint32_t sizeOfBufToDeSerTo = 0U;
    const uint8_t *pBufToDeSerTo = GetSerializableDataBuffer(sizeOfBufToDeSerTo);
    if((pBufToDeSerTo != nullptr) && (entryDataSize <= sizeOfBufToDeSerTo))
    {
      std::memcpy((void*)pBufToDeSerTo,(void*)pEntryData,entryDataSize);
      if(!Deserialize())
      {
        LogMsg(LogPrioError, "ERROR: LogEntry::PopulateMeFrom failed to deserialize Errno: 0x%x (%s)",errnoGet(),strerror(errnoGet()));
      }
    }
    else
    {
      LogMsg(LogPrioError, "ERROR: LogEntry::PopulateMeFrom Invalid serializations buffers 0x%x %u %u Errno: 0x%x (%s)",pBufToDeSerTo,entryDataSize,sizeOfBufToDeSerTo,errnoGet(),strerror(errnoGet()));
    }
  }
  else
  {
    LogMsg(LogPrioError, "ERROR: LogEntry::PopulateMeFrom Invalid SyncMsg buffers 0x%x %u %u  Errno: 0x%x (%s)",pEntryData,entryDataSize,sizeof(LogEntryData),errnoGet(),strerror(errnoGet()));
  }
}


void LogEntry::Print() const
{
  LogMsg(LogPrioInfo,"LogEntry: Op %u View %u Rid %u %s %s",m_entryData.opNumber,m_entryData.viewNumber,m_entryData.rid, m_entryData.isCommited ? "COMMITED" : "NOT COMMITED", m_entryData.isUpcallDone ? "UPCALL MADE" : "UPCALL TBD");
  if(m_pClientMsg)
  {
    m_pClientMsg->Print();
  }
  else
  {
    LogMsg(LogPrioInfo,"LogEntry: No valid ClientMessage");
  }
}

bool LogEntry::IsValidNextOpAndViewNumber(const unsigned int opNumber, const unsigned int viewNumber)
{
  bool isValidNextEntry = false;
  if(opNumber == m_entryData.opNumber +1)
  {
    if(viewNumber >= m_entryData.viewNumber)
    {
      isValidNextEntry = true;
    }
  }
  return isValidNextEntry;
}
