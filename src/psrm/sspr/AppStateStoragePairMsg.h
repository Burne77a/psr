#ifndef SSPR_APPSTATE_STORAGE_MSG_REG_H
#define SSPR_APPSTATE_STORAGE_MSG_REG_H
#include "AppStateStoragePair.h"
#include "ISerializable.h"
#include <memory>

class AppStateStoragePairMsg : public ISerializable
{
  public:
    enum class MsgType
    {
      AddPairing = 0,
      RemovePairing
    };
  private:
    struct AppStateStoragePairMsgData
    {
      MsgType type;
      uint32_t payloadSize;
    };
  public:
    AppStateStoragePairMsg(const uint8_t * pBuffer, const uint32_t size);
    AppStateStoragePairMsg(const std::shared_ptr<ISerializable>& payload, MsgType type );
    ~AppStateStoragePairMsg() = default;
    bool IsAddRequest() const {return m_data.type == MsgType::AddPairing;}
    bool IsRemoveRequest() const {return m_data.type == MsgType::RemovePairing;}
    std::shared_ptr<AppStateStoragePair> GetAppStateStoragePairPayload() const;
    //ISerializable
    const uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override ;
    
    static const std::string GetMsgTypeAsString(const MsgType type);
    void Print() const;
    
  private:
    AppStateStoragePairMsgData m_data;
    std::shared_ptr<ISerializable> m_pPayload{nullptr};
    mutable std::vector<uint8_t> m_serializedDataInclMsgAndPayload;
    const unsigned int MAX_PAYLOAD_SIZE = 512;
};
#endif //SSPR_APPSTATE_STORAGE_MSG_REG_H
