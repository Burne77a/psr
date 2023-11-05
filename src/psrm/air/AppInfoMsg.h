#ifndef AIR_APPINFOMSG_H
#define AIR_APPINFOMSG_H
#include "AppInfo.h"
#include "ISerializable.h"
#include <memory>

class AppInfoMsg : public ISerializable
{
  public:
    enum class MsgType
    {
      AddRequest = 0,
      RemoveRequest
    };
  private:
    struct AppInfoMsgData
    {
      MsgType type;
      uint32_t payloadSize;
    };
  public:
    AppInfoMsg(std::shared_ptr<ISerializable>& payload, MsgType type );
    ~AppInfoMsg() = default;
    std::shared_ptr<AppInfo> GetAppInfoPayload() const;
    //ISerializable
    const uint8_t *  Serialize(uint32_t &size) const override;
    const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
    bool Deserialize() override ;
    
    static const std::string GetMsgTypeAsString(const MsgType type);
    void Print() const;
    
  private:
    AppInfoMsgData m_data;
    std::shared_ptr<ISerializable> m_pPayload{nullptr};
    mutable std::vector<uint8_t> m_serializedDataInclMsgAndPayload;
    const unsigned int MAX_PAYLOAD_SIZE = 1024;
};
#endif //AIR_APPINFOMSG_H
