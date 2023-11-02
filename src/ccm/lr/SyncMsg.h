#ifndef CCM_LR_SYNCMSG_H
#define CCM_LR_SYNCMSG_H
#include "ISerializable.h"
#include <memory>
#include <vector>

class SyncMsg : public ISerializable
{
  public:
    enum class MsgType
      {
        RequestLog = 0,
        LogEntry
      };   
  private:
    struct SyncMsgData
    {
      MsgType type{MsgType::RequestLog};
      int32_t requesterId{0U};
      uint32_t totalNumberOfEntries{0U};
      uint32_t currentEntryIndex{0U};
      uint32_t payloadSize{0U};     
    };
    
  public:
    static std::shared_ptr<SyncMsg> CreateSyncMsgFromLogEntry(const int requesterId,const unsigned int numberOfEntries, const ISerializable &entry, const unsigned int currentEntryIndex);
    SyncMsg(const int requsterId,const unsigned int numberOfEntries, const ISerializable &entry, const unsigned int currentEntryIndex);
    SyncMsg(const MsgType type, const int idOfRequester);
    SyncMsg();
    ~SyncMsg();
    bool IsRequest() const {return m_data.type == MsgType::RequestLog;}
    bool IsEntry() const {return m_data.type == MsgType::LogEntry;}
    int GetIdOfRequester() const {return m_data.requesterId;}
    uint32_t GetCurrentIndex() const {return m_data.currentEntryIndex;}
    uint32_t GetTotalNumberOfEntries() const {return m_data.totalNumberOfEntries;}
    uint8_t * GetDeserializedPayload(uint32_t& size) const;
    //ISerializable
    const uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override;
    static const std::string GetMsgTypeAsString(const MsgType type);
    
    void Print() const;
      
  private:
    mutable std::vector<uint8_t>  m_serializedDataInclPayload;
    const uint32_t MAX_PAYLOAD_SIZE = 1200; //TODO: Not nice, but works for now. Ensure that if the ClientMessage size change, this must change
    SyncMsgData m_data;
};


#endif//CCM_LR_SYNCMSG_H
