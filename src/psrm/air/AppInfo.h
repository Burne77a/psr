#ifndef AIR_APPINFO_H
#define AIR_APPINFO_H
#include "ISerializable.h"
#include <memory>
class AppInfo : public ISerializable
{
  public:
    static const unsigned int INVALID_VALUE{0U};
    static const unsigned int INVALID_APP_ID{INVALID_VALUE};
  private:
    struct AppInfoData
    {
     unsigned int appId{INVALID_APP_ID};
     unsigned int primaryNodeId{INVALID_VALUE};
     unsigned int periodInMs{INVALID_VALUE};
     unsigned int bytesToSyncEachPeriod{0U}; 
    };
  public: 
    static std::shared_ptr<AppInfo> CreateFromRawPtr(const uint8_t * pBuffer, const uint32_t size);
    AppInfo();
    ~AppInfo() = default;
    //ISerializable
     const uint8_t *  Serialize(uint32_t &size) const override;
     const uint8_t * GetSerializableDataBuffer(uint32_t &size) const override;
     bool Deserialize() override;
   void Print() const; 
  private:
     AppInfoData m_data;
};

#endif //AIR_APPINFOENTRY_H
