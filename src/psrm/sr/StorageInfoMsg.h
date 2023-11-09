#ifndef AIR_STORAGEINFOMSG_H
#define AIR_STORAGEINFOMSG_H
#include "StorageInfo.h"
#include "ISerializable.h"
#include <memory>

class StorageInfoMsg : public ISerializable
{
  public:
    enum class MsgType
    {
      AddRequest = 0,
      RemoveRequest
    };
  private:
    struct StorageInfoMsgData
    {
      MsgType type;
      uint32_t payloadSize;
    };
  public:
    StorageInfoMsg(const uint8_t * pBuffer, const uint32_t size);
    StorageInfoMsg(const std::shared_ptr<ISerializable>& payload, MsgType type );
    ~StorageInfoMsg() = default;
    bool IsAddRequest() const {return m_data.type == MsgType::AddRequest;}
    bool IsRemoveRequest() const {return m_data.type == MsgType::RemoveRequest;}
    std::shared_ptr<StorageInfo> GetStorageInfoPayload() const;
    //ISerializable
    const uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override ;
    
    static const std::string GetMsgTypeAsString(const MsgType type);
    void Print() const;
    
  private:
    StorageInfoMsgData m_data;
    std::shared_ptr<ISerializable> m_pPayload{nullptr};
    mutable std::vector<uint8_t> m_serializedDataInclMsgAndPayload;
    const unsigned int MAX_PAYLOAD_SIZE = 512;
};
#endif //AIR_STORAGEINFOMSG_H
