#ifndef CCM_LR_LOGENTRY_H
#define CCM_LR_LOGENTRY_H
#include "ISerializable.h"
#include "LogReplicationMsg.h"
#include "SyncMsg.h"
#include "../csa/ClientMessage.h"
#include "../csa/ClientRequestId.h"
#include <memory>

class LogEntry : public ISerializable
{
  private:
    struct LogEntryData
    {
      unsigned int opNumber{0U};
      unsigned int viewNumber{0U};
      ClientRequestId rid{}; 
      unsigned int payloadSize{0U};
      bool isCommited{false};
      bool isUpcallDone{false};
    };
  public:
    static std::unique_ptr<LogEntry> CreateEntry(const LogReplicationMsg &msg);
    static std::unique_ptr<LogEntry> CreateEntry(const SyncMsg &msg);
    LogEntry(const unsigned int opNumber, const unsigned int viewNumber, ClientRequestId &rid, std::shared_ptr<ClientMessage> pClientMsg);
    LogEntry();
    ~LogEntry() = default;
    bool IsValidNextOpAndViewNumber(const unsigned int opNumber, const unsigned int viewNumber);
    unsigned int GetOpNumber() const {return m_entryData.opNumber;}
    unsigned int GetViewNumber() const {return m_entryData.viewNumber;}
    ClientRequestId GetReqId() const{return m_entryData.rid;}
    void SetCommitted() {m_entryData.isCommited = true;}
    void SetUpCallDone() {m_entryData.isUpcallDone = true;}
    bool IsCommited() const {return m_entryData.isCommited;}
    bool IsUpcallDone() const {return m_entryData.isUpcallDone;} 
    std::shared_ptr<ClientMessage> GetClientMsg()const{return m_pClientMsg;}
    
    //ISerializable
    const uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    
    void Print() const;
    
  private:
    void PopulateMeFrom(const SyncMsg &msg);
    const uint32_t MAX_PAYLOAD_SIZE = 1200; //TODO: Not nice, but works for now. Ensure that if the ClientMessage size change, this must change
    LogEntryData m_entryData;
    std::shared_ptr<ClientMessage> m_pClientMsg{nullptr};
    mutable std::vector<uint8_t> m_serializedDataInclPayload;
};

#endif //CCM_LR_LOGENTRY_H
